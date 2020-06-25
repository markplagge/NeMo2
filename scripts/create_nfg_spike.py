import json
import multiprocessing
import queue
import sqlite3
import threading
import time
import warnings
from pathlib import Path

import click
import dask
import dask.dataframe as dd
import pandas as pd
import tqdm
from dask.distributed import Client
from numba import jit
from sqlalchemy import create_engine
from JSReader import JSReader
import msgpack
from io import BytesIO

class Counter(object):
    def __init__(self, initval=0):
        self.val = multiprocessing.Value('i', initval)
        self.lock = multiprocessing.Lock()
        x = 32
    def increment(self):
        with self.lock:
            self.val.value += 1

    def value(self):
        with self.lock:
            return self.val.value

def func(counter):
    for i in range(50):
        time.sleep(0.01)
        counter.increment()


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
    counter = Counter()
    def __init__(self,nworkers, client, nfg_file, model_id, nid=1, db_path=Path("./model_data.sqlite"), table_name="model",
                 db_user='plaggm', do_sql=True, out_json_file='./out.json', out_csv_file='./out.csv',buffer_size=2048):
        super().__init__(None, None, 0, db_path, table_name, do_sql, use_pandas=False)
        self.buffer_size = buffer_size
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
        frb_m = min(buffer_size, nworkers * 2)
        self.file_read_buffer = multiprocessing.Queue(maxsize=frb_m * 10)

        self.line_buffer = multiprocessing.Queue()
        self.json_result_buffer = multiprocessing.Queue()


        #self.nworkers = (multiprocessing.cpu_count() // 2) + 2
        self.nworkers = nworkers
        self.json_archive_q = multiprocessing.Queue()
        self.end_sql_q = multiprocessing.Queue()
        self.file_out_buffer = multiprocessing.Queue()
        self.out_json_name = Path(out_json_file)
        self.out_csv_name = Path(out_csv_file)

        self.completed_dataframe_q = multiprocessing.Queue(maxsize=100000)
        self.done_workers = multiprocessing.Queue()
        self.lr_buffer = 1000

    def line_reader(self,th=False):
        buffer = []
        nb = 0
        line_buffer = self.buffer_size * 2
        def dsc():
            return f'Read - lines : {nb}/{len(buffer)}'
        with open(self.nfg_file, 'r') as nfg:
            # for l in tqdm.tqdm(nfg, desc=f'reading lines : {nb}/{len(buffer)}', position=1, leave=False):
            with tqdm.tqdm(desc=dsc(), position=1) as pbar:
                for l in nfg:
                    if "{" in l:
                        buffer.append(l)

                    if len(buffer) > line_buffer:
                        # tqdm.tqdm.write(f"Buffer {nb} queue up")
                        bx = [l for l in buffer]
                        self.file_read_buffer.put(bx)
                        buffer = []
                        nb += 1
                    pbar.desc = f"reading lines : {nb}/{len(buffer)}: BS: {line_buffer}"
                    pbar.update(0)
                    if th:
                         time.sleep(0.000001)
                #pbar.desc =  f"reading lines : {nb}/{len(buffer)}: BS: {line_buffer} - DONE!"""

                pbar.desc = dsc() + " -- DONE --"
                pbar.update(0)

        if buffer:
            self.file_read_buffer.put(buffer)
        for i in range(0,self.nworkers + 5):
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
                pbar.update(0)
                try:
                    rez = self.json_result_buffer.get_nowait()

                    if isinstance(rez, str) and "DONE" in rez:
                        check_done = True
                    else:
                        local_buffer.append(rez)
                except:
                    pbar.update(0)
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
            pbar.update(0)
            while not self.json_result_buffer.empty():
                rez = self.json_result_buffer.get()
                if not isinstance(rez, str):
                    self.write_sql(rez)
                    self.completed_dataframe_q.put(rez)
                pbar.update(0)

            pbar.desc = 'SQL thread Second Cleanup'
            pbar.update(0)
            if local_buffer:
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
                # if isinstance(vx, list):
                #     vx = ",".join([str(tx) for tx in vx])
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
    @jit
    def json_reader_work(self, ndone, line, js_buf):

        if "{" in line:
            # rx = r"(TN_?\d*?_\d*?\s*?=)"
            # match = re.findall(rx, line)[0].replace("=", "").rstrip()
            k = self.jit_lp(line)
            ndone += 1
            kd = list(k.keys())[0]
            kv = k[kd]
            js_buf[kd] = kv
            # self.file_out_buffer.put({kd: kv})

        return js_buf

    def json_reader(self, i,th=False):


        # with tqdm.tqdm(position=i, leave=False, desc="Reading JSON") as pbar:
        ndone = 0
        js_buf = {}
        buflen = self.buffer_size // 2
        def dvx(c):
            return f'Worker {i - 2} processing'
        while True:

            line_arr = self.file_read_buffer.get()
            if isinstance(line_arr, str) and "DONE" in line_arr:
                self.file_read_buffer.put("DONE")
                self.done_workers.put("DONE")
                break
            self.counter.increment()
            ldx = 0
            max_ldx = max(self.buffer_size // 100, 10)
            ct = self.counter.value()
            with tqdm.tqdm(total = len(line_arr), position = i, desc=dvx(ct), leave=False) as pbar:
            # for line in tqdm.tqdm(line_arr, position=i,
            #                       desc=dvx(),
            #                       leave=False):
                for line in line_arr:
                    pbar.update(0)
                    js_buf = self.json_reader_work(ndone, line, js_buf)
                    if len(list(js_buf.keys())) > buflen:
                        k = self.df_gen(js_buf)
                        self.json_result_buffer.put(k)
                        js_buf = {}

                    # if ldx > max_ldx:
                    #     ct = self.counter.value()
                    #     ldx = 0

                    pbar.desc = dvx(ct)
                    pbar.update(1)
                    ldx += 1


        if len(list(js_buf.keys())):
            k = self.df_gen(js_buf)
            self.json_result_buffer.put(k)
        if th:
            time.sleep(0.4)
        self.json_result_buffer.put("DONE")
        self.done_workers.put("DONE")
        #click.secho("File Read Done", fg="bright_green", underline=True)

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
                        bbar.update(0)
                    while not self.completed_dataframe_q.empty():
                        result_dataframes.append(self.completed_dataframe_q.get())

                pbar.desc = f"{dxd}{len(file_out_buffer)}"
                pbar.update(0)
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
import tempfile


class ParJSConv(DaskJS):
    file_worker_cleanup = multiprocessing.RLock()
    def __init__(self, *args, **kwargs):
        super(ParJSConv, self).__init__(*args, **kwargs)
        self.n_workers = self.nworkers
        self.js_buff_q = multiprocessing.Queue()
        self.n_done_workers = 0
        self.ttl_workers = 0

    def json_read_jl(self, line):
        js_buf = self.json_reader_work(0, line, {})
        self.js_buff_q.put(js_buf)

    def append_json(self, df, out_file_handle, orient="index"):

        if not isinstance(df, pd.DataFrame):
            click.secho(f"{'*' * 30}\nGOT WEIRD VALUE IN WRITER:\n {df}", bold=True, underline=True, fg="red")

        js = df.to_json( orient='index')

        js = js.lstrip("{")
        js = js.split("},")

        js = "},\n".join(js)
        js = js.replace("}}","},")


        # js = js.replace("}}","},\n")
        # js = js.rstrip("}")

        out_file_handle.write(js.encode())

    def write_msgpack(self,df, buf = BytesIO()):
        for d in df.itertuples():
            dc = d._asdict()
            buf.write(msgpack.packb(dc, use_bin_type=True))


    def json_writer(self,th=False):
        mpfile = self.out_json_name.parent / self.out_json_name.name.replace(".json",".mb")
        with tqdm.tqdm(desc="Writer running: waits: 0 / writes: 0", position=2, leave=False) as pbar:
            with tqdm.tqdm(desc="Line Q TTL", position=multiprocessing.cpu_count() + 2, leave=True) as ttl_bar:
                big_data = None
                #buf = BytesIO()
                with open(mpfile, "wb") as buf:
                    with open(self.out_json_name, 'wb+') as outf:
                        outf.write(b'{')
                        niter = 0
                        nwrites = 0
                        nd = self.counter.value()
                        while True:
                            if th:
                                time.sleep(0.001)
                            try:
                                df = self.json_result_buffer.get(timeout=2)
                                if not isinstance(df, str):
                                    if isinstance(df, pd.DataFrame):
                                    # js_buff.update(dct)
                                        self.append_json(df,outf)
                                        self.write_msgpack(df,buf)
                                        big_data = df if big_data is None else pd.concat([df,big_data])
                                        nwrites += 1
                                    else:
                                        click.secho(f"{'*'*30}\nGOT WEIRD VALUE IN WRITER:\n {df}",bold=True,underline=True, fg="red")
                            except queue.Empty:
                                time.sleep(0.5)
                            niter += 1
                            counter_val = self.counter.value()
                            if counter_val == nd:
                                ttl_bar.update(0)
                            else:
                                while counter_val > nd:
                                    nd += 1
                                    ttl_bar.update(1)

                            pbar.desc = f"Writer Running: waits:{niter}|writes:{nwrites}|ttlLQ:{self.counter.value()}"
                            pbar.update()

                            # try:
                            #     dd = self.done_workers.get_nowait()
                            #     nd += 1
                            # except queue.Empty:
                            #     pass
                            # if nd >= self.n_workers:
                            #     break
                            if self.file_worker_cleanup.acquire(block=False):
                                break

                        while(self.json_result_buffer.empty() == False):
                            df = self.json_result_buffer.get()
                            if isinstance(df, pd.DataFrame):
                                self.write_msgpack(df,buf)
                                self.append_json(df, outf)
                                big_data = pd.concat([big_data,df])
                                #nwrites += 1
                                pbar.desc = f"Writer Finishing - writes: {nwrites}"


                                #pbar.update(0)
                        outf.seek(-1,1)
                        outf.write(b" }\n")
                        pbar.desc = f"Writer Writing MP: {nwrites}"
                        pbar.update(0)
                        with open(self.out_json_name.parent / 'full_dump.mp', 'wb') as bigf:
                            self.write_msgpack(big_data,bigf)

    def loader(self):
        with open(self.nfg_file, 'r') as nfg:
            for l in nfg:
                if "{" in l:
                    yield l

    def check_worker_done(self):
        try:
            wd = self.done_workers.get_nowait()
            self.done_workers.put(wd)
            self.n_done_workers += 1

        except:
            pass
        return (self.n_done_workers >= self.ttl_workers)

    def parse_file(self,debug=False):
        self.file_worker_cleanup.acquire()
        #reader = threading.Thread(target=self.line_reader,args=(True,))
        if debug:
            self.buffer_size = 9000000
            self.lr_buffer = self.buffer_size
            self.line_reader()
            self.json_reader(1)
            self.json_writer()
        reader = multiprocessing.Process(target=self.line_reader)
        workers = []
        i = 3
        while self.nworkers > 1:
            workers.append(multiprocessing.Process(target=self.json_reader, args=(i,)))
            i += 1
            self.nworkers -= 1
        self.n_workers = len(workers)
        click.secho(f"Started {len(workers)} workers",fg="green")


        writer = multiprocessing.Process(target=self.json_writer)
        #writer = threading.Thread(target=self.json_writer,args=(True,))

        js_buf = {}
        dxd = "Generating Bags (MT) - FP: "
        self.ttl_workers = len(workers)
        with tqdm.tqdm(position=0, desc=dxd, leave=False) as pbar:

            [worker.start() for worker in workers]
            reader.start()
            writer.start()
            niter = -1
        print(self.json_result_buffer.empty())
        click.secho("Join Reader", fg="bright_blue")
        reader.join()
        click.secho("Line Worker Join", fg="bright_blue")
        #[self.file_read_buffer.put("DONE") for _ in workers]
        [self.done_workers.put("DONE") for _ in workers]
        with tqdm.tqdm(workers, desc='worker joins',position=0,leave=False) as pbar:
            rd = True
            while rd:
                rs = []
                for w in workers:
                    rs.append(w.is_alive())
                rd = all(rs)
                time.sleep(2)
                pbar.update(0)


        [w.join() for w in workers]
        click.secho("Writer Join---------------------------",fg="magenta")
        self.file_worker_cleanup.release()

        writer.join()



def connect_dask_client(host='localhost', port="8786", nprocs=0, nthreads=0):
    if host is None:
        if nprocs == 0:
            nprocs = multiprocessing.cpu_count() - 1
        client = Client()
    else:
        client = Client(f"{host}:{port}")
    return client



def convert_spike_to_json(sqlite_json_filename,client=None):
    rr = "sqlite"
    assert(isinstance(sqlite_json_filename,Path))
    out_file = sqlite_json_filename.name.split(".")[0]
    out_file = Path(out_file +".json")

    click.secho(f"Starting Spike conversion. Reading {sqlite_json_filename} into {out_file}\n",fg="blue")
    con = sqlite3.connect(str(sqlite_json_filename.absolute()))
    df = pd.read_sql_query("SELECT * from input_spikes",con)
    jss = df.to_json( orient='records')
    jss = jss.replace("},", "},\n")
    out_file.write_text(jss)
    click.secho("Done writing spike json", fg="green")


## Read JSON nfg file and
@click.command()
@click.option('--no-dask', is_flag=True, default=False, help="Use DASK for df processing?")
@click.option("--dask-host", default="cci-triton.duckdns.org")
@click.option("--local-dask", is_flag=True, default=False)
@click.option('--dask-port', default="8786")
@click.option("--model-name")
@click.option("--only-json", default=False, is_flag=True, help="Save result to JSON only.")
@click.option("--out-js", default="./new_js.json", help="Json output file")
@click.option("--n-wkrs", default=20)
@click.option("--buffer-size", default=4096, help="Parser process buffer size")
@click.option("--debug",default=False, is_flag=True)
@click.argument('nfg-file', type=click.Path(exists=True, dir_okay=False))
@click.argument('spike-file', type=click.Path(exists=True, dir_okay=False))
@click.argument('new-sqlite')
def cli(no_dask, dask_host, local_dask, dask_port, model_name, only_json, out_js, n_wkrs, buffer_size, debug, nfg_file, spike_file, new_sqlite):
    # spike = Path(spike_file)
    click.clear()
    if "sql" not in spike_file:
        click.secho(f"Got {spike_file} as spike db - does not look like a sqlite3 db",fg="red")

        click.confirm(click.style("Continue", bold=True, fg="red"),abort=True)

    if not no_dask:
        if local_dask:
            dask_host = None
        client = connect_dask_client(dask_host, dask_port)
    else:
        client = None
    nfg = Path(nfg_file)

    if only_json:
        click.secho("JSON ONLY", fg="green")
        reader = ParJSConv(n_wkrs,client, nfg, model_name, nid=2, db_path=new_sqlite, out_json_file=out_js,buffer_size=buffer_size)
    else:
        reader = DaskJS(n_wkrs,client, nfg, model_name, nid=2, db_path=new_sqlite,buffer_size=buffer_size)

    click.secho(f"created reader for model {reader.model_id}.\nSaving to {new_sqlite}", fg="green")
    click.secho("Starting up SPIKE conversion...",fg="green")
    spike_file = Path(spike_file)
    spike_worker = multiprocessing.Process(target=convert_spike_to_json,args=(spike_file,))
    spike_worker.start()
    if debug:
        reader.parse_file(debug=True)
    else:
        reader.parse_file()
    click.secho("Complete - Waiting on spike",fg="green")
    spike_worker.join()


if __name__ == '__main__':
    cli()
