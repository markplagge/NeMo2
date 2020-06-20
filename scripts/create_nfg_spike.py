import click
import dask
import dask.dataframe as dd
import multiprocessing
import pandas as pd
import threading
import time
import tqdm
import warnings
from JSReader import JSReader
from dask.distributed import Client
from numba import jit
from pathlib import Path
from sqlalchemy import create_engine

warnings.filterwarnings('ignore')


class ModelInfo():
    model_path = Path("./model.nfg")
    spike_path = Path("./spikes.sqlite")
    cores = 0
    neuron_weights = 0
    neurons_per_core = 0
    name = ''

    def _get_global(self):
        with open(str(self.model_path.absolute()), 'r') as mf:
            for l in mf:
                if "{" not in l:
                    dat = [fx.rstrip().lstrip() for fx in l.split('=')]
                    dat[1] = int(dat[1])
                    self.__dict__[dat[0]] = dat[1]
                else:
                    break

    def __init__(self, model_path, spike_path, model_name=None, model_id=0):
        assert isinstance(model_path, Path)
        assert isinstance(spike_path, Path)

        if model_name is None:
            model_name = model_path.name
            model_name = model_name.split('.')[0]
        self.model_name = model_name

        self.model_id = model_id

        self.model_path = model_path
        self.spike_path = spike_path
        self._get_global()


class DaskJS(JSReader):
    def __init__(self, client, nfg_file, model_id, nid=1, db_path=Path("./model_data.sqlite"), table_name="model",
                 db_user='plaggm', do_sql=True, out_json_file='./out.json', out_csv_file='./out.csv'):
        super().__init__(None, None, 0, db_path, table_name, do_sql, use_pandas=False)
        self.nfg_file = nfg_file
        self.model_id = model_id
        if isinstance(db_path, str):
            db_path = Path(db_path)

        self.sqlite_uri = f"sqlite:///{db_path.absolute()}"
        self.conn = create_engine(self.sqlite_uri)
        self.use_odo = False
        self.nid = nid
        self.dfs = []
        self.client = client

        self.file_read_buffer = multiprocessing.SimpleQueue()

        self.line_buffer = multiprocessing.Queue()
        self.json_result_buffer = multiprocessing.Queue()

        self.buffer_size = 4096
        self.nworkers = (multiprocessing.cpu_count() // 2) + 2

        self.json_archive_q = multiprocessing.Queue()

        self.end_sql_q = multiprocessing.Queue()

        self.file_out_buffer = multiprocessing.Queue()

        self.out_json_name = Path(out_json_file)
        self.out_csv_name = Path(out_csv_file)

        self.completed_dataframe_q = multiprocessing.Queue(maxsize=100000)

    def line_reader(self):
        buffer = []
        nb = 0
        with open(self.nfg_file, 'r') as nfg:
            # for l in tqdm.tqdm(nfg, desc=f'reading lines : {nb}/{len(buffer)}', position=1, leave=False):
            with tqdm.tqdm(desc=f'reading lines : {nb}/{len(buffer)}', position=2) as pbar:
                for l in nfg:
                    if "{" in l:
                        buffer.append(l)

                    if len(buffer) > self.buffer_size:
                        # tqdm.tqdm.write(f"Buffer {nb} queue up")
                        pbar.desc = f"reading lines : {nb}/{len(buffer)}"
                        bx = [l for l in buffer]
                        self.file_read_buffer.put(bx)
                        buffer = []
                        time.sleep(.5)
                        nb += 1
                    pbar.update(1)

        if buffer:
            self.file_read_buffer.put(buffer)
        self.file_read_buffer.put("DONE")

    def write_sql(self, data):
        if isinstance(data, dask.dataframe.DataFrame):
            data = data.compute()
        assert isinstance(data, pd.DataFrame)
        data.to_sql('model', con=self.conn, index=True, if_exists='append')

    def sql_thread(self):
        with tqdm.tqdm(desc=f"Sql to {self.sqlite_uri}", position=self.i + 1, leave=True) as pbar:
            local_buffer = []
            num_proc = 0
            biff_size = max(10, multiprocessing.cpu_count() // 8)
            num_writes = 0
            check_done = False
            while True:
                pbar.update(1)
                try:
                    rez = self.json_result_buffer.get_nowait()

                    if isinstance(rez, str) and "DONE" in rez:
                        check_done = True
                    else:
                        local_buffer.append(rez)
                except:
                    pbar.update(1)
                    time.sleep(2)

                if check_done and not self.end_sql_q.empty():
                    break
                if len(local_buffer) > biff_size:
                    dbx = pd.concat(local_buffer)
                    self.write_sql(dbx)
                    local_buffer = []
                    num_proc += biff_size
                    time.sleep(0.001)
                    num_writes += 1

                    ## save the resulting dataframe:
                    self.completed_dataframe_q.put(dbx)

                pbar.desc = f"Sql to {self.sqlite_uri} - N: {num_proc}:{num_writes}/{len(local_buffer)}"
                time.sleep(1)
            # CLEANUP
            pbar.desc = f"SQL Thread First Cleanup"
            pbar.update(1)
            while not self.json_result_buffer.empty():
                rez = self.json_result_buffer.get()
                if not isinstance(rez, str):
                    self.write_sql(rez)
                    self.completed_dataframe_q.put(rez)
                pbar.update(1)

            pbar.desc = 'SQL thread Second Cleanup'
            pbar.update(1)
            if len(local_buffer) > 0:
                dbx = pd.concat(local_buffer)
                self.write_sql(dbx)

    @jit
    def jit_lp(self, line):
        k = self.parse_json_line(line)
        # if not isinstance(k, dict):
        #     k = demjson.decode(k)
        # k['core_uid'] = match
        for key in k.keys():
            new_dict = {}
            tn_c = k[key]
            for k2 in tn_c.keys():
                vx = tn_c[k2]
                if isinstance(vx, list):
                    vx = ",".join([str(tx) for tx in vx])
                new_dict[k2] = vx
            k[key] = new_dict
            break

        return k

    @jit
    def df_gen(self, js_buf):
        # k = pd.concat([pd.DataFrame.from_dict(js,orient='index') for js in js_buf])
        k = pd.DataFrame.from_dict(js_buf, orient="index")
        k.index.name = "core_uid"
        # to_fix = [
        #     ['core_id', 'coreID'],
        #     ['local_id', 'localID'],
        #     ['core_type', 'type'],
        #     ['synaptic_connectivity', 'synapticConnectivity'],
        #     ['sigma_g', 'sigmaG'],
        #     ['s', 'S'],
        #     ['tm', 'TM'],
        #     ['vr', 'VR'],
        #     ['sigma_vr', 'sigmaVR'],
        #     ['sigma_lambda', 'sigma_lmbda'],
        #     ['lambda', 'lmbda'],
        #     ['signal_delay', 'signalDelay'],
        #     ['dest_core', 'destCore'],
        #     ['dest_local', 'destLocal'],
        #     ['output_neuron', 'outputNeuron'],
        #     ['self_firing', 'selfFiring']
        # ]
        # for fix_pair in to_fix:
        #     new = fix_pair[0]
        #     old = fix_pair[1]
        #     k[new] = k[old]
        #     k = k.drop(columns=[old])

        k['model_id'] = self.model_id
        return k

    def json_reader(self, i):
        time.sleep(1)
        print(f"POSITION {i}")
        # with tqdm.tqdm(position=i, leave=False, desc="Reading JSON") as pbar:
        ndone = 0
        js_buf = {}
        buflen = self.buffer_size // 2

        while True:

            line_arr = self.file_read_buffer.get()
            if isinstance(line_arr, str) and "DONE" in line_arr:
                self.file_read_buffer.put("DONE")
                break
            for line in tqdm.tqdm(line_arr, position=i, desc=f'p:{i - 2} - line processing:{ndone}', leave=False):
                if "{" in line:
                    # rx = r"(TN_?\d*?_\d*?\s*?=)"
                    # match = re.findall(rx, line)[0].replace("=", "").rstrip()
                    k = self.jit_lp(line)
                    ndone += 1
                    kd = list(k.keys())[0]
                    kv = k[kd]
                    js_buf[kd] = kv
                    # self.file_out_buffer.put({kd: kv})
                if len(list(js_buf.keys())) > buflen:
                    k = self.df_gen(js_buf)
                    self.json_result_buffer.put(k)
                    js_buf = {}

        if len(list(js_buf.keys())):
            k = self.df_gen(js_buf)
            self.json_result_buffer.put(k)

        self.json_result_buffer.put("DONE")
        click.secho("File Read Done", fg="bright_green", underline=True)

    def parse_file(self):

        def bag_filter(element):
            return isinstance(element, dict) or isinstance(element, pd.DataFrame)

        reader = multiprocessing.Process(target=self.line_reader)
        workers = []
        i = 3
        while self.nworkers > 1:
            workers.append(multiprocessing.Process(target=self.json_reader, args=(i,)))
            i += 1
            self.nworkers -= 1
        # workers = [multiprocessing.Process(target=self.json_reader,args=(i,)) for i in range(1, multiprocessing.cpu_count() - 2)]
        self.i = i
        writer = threading.Thread(target=self.sql_thread)

        import time
        qbx = []
        file_out_buffer = []
        result_dataframes = []
        dxd = "Generating Bags (MT) - FP: "
        with tqdm.tqdm(position=0, desc=dxd, leave=False) as pbar:
            reader.start()
            [worker.start() for worker in workers]
            writer.start()
            while True:
                if not writer.is_alive():
                    click.secho("Writer exit", fg='bright_blue')
                    break

                time.sleep(1)
                with tqdm.tqdm(position=1, desc="Working file queues from main thread", leave=False) as bbar:
                    while not self.file_out_buffer.empty():
                        file_out_buffer.append(self.file_out_buffer.get())
                        bbar.update(1)
                    while not self.completed_dataframe_q.empty():
                        result_dataframes.append(self.completed_dataframe_q.get())

                pbar.desc = f"{dxd}{len(file_out_buffer)}"
                pbar.update(1)
        print(self.json_result_buffer.empty())
        click.secho("Join Reader", fg="bright_blue")
        reader.join()
        click.secho("Line Worker Join", fg="bright_blue")
        for p in tqdm.tqdm(workers, desc='worker join1'):
            self.file_read_buffer.put("DONE")
            p.join()
        click.secho("Ending sql writer", fg="bright_blue")
        self.end_sql_q.put("DONE")
        writer.join()

        click.secho("Saving giant json", fg='blue')
        bigdf = dd.concat(qbx)

        save_df = self.client.compute(dd.to_json, bigdf, "full_model_fix.json", orient='index')
        save_df.result()

        # bag = db.from_sequence(dat,npartitions=2).map(json_reader).filter(bag_filter)
        # bag = self.client.compute(bag)
        # self.dfs.append(bag)

        # for df in tqdm.tqdm(as_completed(self.dfs),position=0,leave=True):
        #     self.write_sql(df)


def connect_dask_client(host='localhost', port="8786", nprocs=0, nthreads=0):
    if host is None:
        if nprocs == 0:
            nprocs = multiprocessing.cpu_count() - 1
        client = Client()
    else:
        client = Client(f"{host}:{port}")
    return client


## Read JSON nfg file and
@click.command()
@click.option('--no_dask', is_flag=True, default=False, help="Use DASK for df processing?")
@click.option("--dask-host", default="cci-triton.duckdns.org")
@click.option("--local-dask", is_flag=True, default=False)
@click.option('--dask-port', default="8786")
@click.option("--model-name")
@click.argument('nfg_file', type=click.Path(exists=True, dir_okay=False))
@click.argument('spike_file', type=click.Path(exists=True, dir_okay=False))
@click.argument('new_sqlite')
def cli(no_dask, dask_host, local_dask, dask_port, model_name, nfg_file, spike_file, new_sqlite):
    # spike = Path(spike_file)
    if not no_dask:
        if local_dask:
            dask_host = None
        nfg = Path(nfg_file)
        client = connect_dask_client(dask_host, dask_port)
        reader = DaskJS(client, nfg, model_name, nid=2, db_path=new_sqlite)

        click.secho(f"created reader for model {reader.model_id}.\nSaving to {new_sqlite}", fg="green")
        reader.parse_file()


if __name__ == '__main__':
    cli()
