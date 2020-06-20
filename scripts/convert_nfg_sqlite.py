# import click
# import json
# import sqlite3
# from pathlib import Path
# from sqlalchemy import create_engine
# from distributed import as_completed
# from joblib import Parallel, delayed
# import pandas as pd
# import tqdm
# import demjson
# import time
# import re
# import multiprocessing
# from multiprocessing import SimpleQueue
#
#
# # myoffcampushousing@gmail.com
# # number too
# # one year leas
# # placesforstudents
#
# class JSReader(multiprocessing.Process):
#     def __init__(self, task_queue, result_queue, nid, db_path, table_name, do_sql=True):
#         multiprocessing.Process.__init__(self)
#         self.do_sql = do_sql
#         self.table_name = table_name
#         self.db_path = db_path
#
#         self.task_queue = task_queue
#         self.result_queue = result_queue
#         self.neu_rem = re.compile(r"(neurons.\=\s?)")
#         self.end_clean_comma = re.compile(r",$")
#         self.end_clean_curl = re.compile(r"}[\s]+(})$")
#         self.fix_tn = re.compile(r"TN_\d+_\d+")
#         self.fix_labs = re.compile(r"\w*(?=\s\=)")
#         self.fix_arrs = re.compile(r"\s({).?[\d,-]+\s(})")
#         self.nid = nid
#         self.sv = 0
#
#     def parse_json_line(self, line):
#         def add_q(match):
#             return f'"{match.group(0)}'
#
#         def rt(match):
#             r = match.group().replace('{', '[').replace('}', ']')
#             return r
#
#         remove_nrn = line.replace('neurons = ', '')
#         remove_nre = re.sub(self.end_clean_comma, "", remove_nrn)
#         remove_end = re.sub(self.end_clean_curl, "}", remove_nre)
#         add_quotes = re.sub(self.fix_labs, add_q, remove_end)
#         fin_quotes = add_quotes.replace("=", ":")
#         fixed_ar = re.sub(self.fix_arrs, rt, fin_quotes)
#         fixed_ar = fixed_ar + "}"
#         fixed_ar = fixed_ar.lstrip()
#         if not fixed_ar.startswith("{"):
#             fixed_ar = "{" + fixed_ar
#         return fixed_ar
#
#     def parse_json_lines(self, lines):
#         jsons = []
#         for l in tqdm.tqdm(lines, position=self.nid + 1, desc=f"Process {self.nid} json lines"):
#             j = self.parse_json_line(l)
#             j = demjson.decode(j)
#             j = pd.DataFrame.from_dict(j, orient="index")
#             jsons.append(j)
#         return jsons
#
#     def parselogic(self, next_task):
#         rr = []
#         if isinstance(next_task, list):
#             jp = self.parse_json_lines(next_task)
#             rr = jp
#         else:
#             jp = self.parse_json_line(next_task)
#             rr.append(jp)
#
#         # rt = [demjson.decode(r) for r in rr]
#         df = pd.concat([pd.DataFrame.from_dict(r) for r in rr])
#
#         # df.to_pickle(f"./p_{self.pid}-n_{self.nid}.dat")
#         self.nid += 1
#         self.result_queue.put(df)
#         self.write_sql(df)
#
#     def write_sql(self, result_df):
#         if not self.do_sql:
#             return
#
#         assert isinstance(result_df, pd.DataFrame)
#         c = sqlite3.connect(self.db_path)
#         result_df.to_sql(self.table_name, c, if_exists="append")
#
#     def run(self, ip=True):
#         proc_name = self.name
#         with tqdm.tqdm(position=self.nid, desc=f"Process {self.nid} working...") as pbar:
#             while True:
#                 next_task = self.task_queue.get()
#                 if next_task is None:
#                     print(f"JSON Parser done")
#                     self.task_queue.task_done()
#                     break
#                 else:
#                     self.parselogic(next_task)
#                 pbar.update(1)
#                 self.task_queue.task_done()
#                 if ip == False:
#                     break
#
#
# global_settings = {}
#
#
# def convert_merge(dest_sql, nfg, spike_sql):
#     global global_settings
#
#     dest_sql = Path(dest_sql)
#     click.secho(f"Loading NFG file into {dest_sql.name}", fg="blue")
#     dest_db = sqlite3.connect(f"{dest_sql.absolute()}")
#     tasks = multiprocessing.JoinableQueue()
#     results = multiprocessing.Queue()
#     nworkers = (multiprocessing.cpu_count() // 2)
#     workers = []
#     pbar_pos = 1
#     for i in range(0, nworkers):
#         click.secho(f"Starting worker {i} - PBAR pos {pbar_pos}", fg="green")
#         worker = JSReader(tasks, results, pbar_pos, str(dest_sql.absolute()), "model", False)
#         workers.append(worker)
#         worker.start()
#         pbar_pos += 2
#     # worker.start()
#     line_cache = []
#     buffer_n = 1024
#     nlines = 0
#     narrs = 0
#     for line in tqdm.tqdm(nfg):
#         line_cache.append(line)
#         if len(line_cache) > buffer_n:
#             tasks.put(line_cache)
#             line_cache = []
#             narrs += 1
#
#         nlines += 1
#     for i in range(0, nworkers):
#         tasks.put(None)
#
#     main_dat = []
#     with tqdm.tqdm(position=0) as pbar:
#         for w in workers:
#             w.join()
#             pbar.update()
#
#     for i in tqdm.tqdm(range(0, narrs, position=0, desc="Gathering Results")):
#         dat = results.get()
#         main_dat.append(dat)
#
#     mdf = pd.concat(main_dat)
#     click.secho("saving to csv file")
#     mdf.to_csv("./concat.csv")
#
#     #     dat = pd.DataFrame(dat)
#     #     dat.to_sql('model',con=str(dest_sql),if_exists="append")
#     #     nlines -=1
#     #     narrs -= 1
#
#     dest_db.execute("""create table model_global
# (
# 	num_cores int,
# 	weights int,
# 	neurons_per_core int,
# 	per_core_id TEXT default "N"
# );""")
#     dest_db.execute(
#         "insert into model_global values ({global_settings['cores']},{global_settings['neuronsPerCore'],{global_settings[neuron_weights]};")
#     return main_dat
#
#     # l = line.rstrip().rstrip(',')
#     # l = l.lstrip('neurons = {')
#     # l = l.replace("=", ":")
#     #     # ld = json.loads(l)
#     #     ld = demjson.decode(l)
#     #     cols = ld.keys()
#
#     print(cols)
#
#
# def get_globals(nfg):
#     line_parser = lambda x: x.replace(' ', '').split('=')
#     file_lp = lambda x: line_parser(x.readline())
#     v1 = file_lp(nfg)
#     v2 = file_lp(nfg)
#     v3 = file_lp(nfg)
#
#     configs = [v1, v2, v3]
#     num_cores = 0
#     neurons_per_core = 0
#     weights_per_neuron = 0
#
#     for cfgi in configs:
#         cn = cfgi[0]
#         cv = int(cfgi[1])
#         if 'cores' in cn:
#             num_cores = cv
#         elif 'neuronsPerCore' in cn:
#             neurons_per_core = cv
#         elif 'neuron_weights' in cn:
#             weights_per_neuron = cv
#         else:
#             click.secho(f"Got {cfgi} config items - undefined.", fg="red")
#     return num_cores, neurons_per_core, weights_per_neuron
#
#
# def divide_chunks(l, n):
#     # looping till length l
#     for i in range(0, len(l), n):
#         yield l[i:i + n]
#
#
# def daskmode(dest, nfg, client="cci-triton.duckdns.org:8786", n_workers=None, db="localhost:5432"):
#     model_id = 0
#     import dask.dataframe as dd
#     import dask.bag as db
#     from distributed import Client
#     if n_workers is None:
#         n_workers = multiprocessing.cpu_count() - 1
#     if client is None or client == "local":
#         client = Client(n_workers=n_workers, direct_to_workers=True)
#     else:
#         client = Client(client)
#     # client = Client(n_workers = multiprocessing.cpu_count() // 2,direct_to_workers=True)
#
#     print(client)
#     wx = JSReader(None, None, 0, None, None, False)
#
#     def json_reader(line):
#         wx = JSReader(None, None, 0, None, None, False)
#         if "{" in line:
#             rx = r"(TN_?\d*?_\d*?\s*?=)"
#             match = re.findall(rx, line)[0].replace("=", "").rstrip()
#             k = wx.parse_json_line(line)
#             k = demjson.decode(k)
#             # k['core_uid'] = match
#             k = pd.DataFrame.from_dict(k, orient="index")
#             k.index.name = "core_uid"
#             to_fix = [
#                 ['core_id', 'coreID'],
#                 ['local_id', 'localID'],
#                 ['core_type', 'type'],
#                 ['synaptic_connectivity', 'synapticConnectivity'],
#                 ['sigma_g', 'sigmaG'],
#                 ['s', 'S'],
#                 ['tm', 'TM'],
#                 ['vr', 'VR'],
#                 ['sigma_vr', 'sigmaVR'],
#                 ['sigma_lambda', 'sigma_lmbda'],
#                 ['lambda', 'lmbda'],
#                 ['signal_delay', 'signalDelay'],
#                 ['dest_core', 'destCore'],
#                 ['dest_local', 'destLocal'],
#                 ['output_neuron', 'outputNeuron'],
#                 ['self_firing', 'selfFiring']
#             ]
#             for fix_pair in to_fix:
#                 new = fix_pair[0]
#                 old = fix_pair[1]
#                 k[new] = k[old]
#                 k = k.drop(columns=[old])
#
#             k['model_id'] = model_id
#
#             return k
#         else:
#             return 0
#
#     def bag_filter(element):
#         return isinstance(element, dict) or isinstance(element, pd.DataFrame)
#
#     bags = []
#     buffer = []
#     bufferlim = 4096 * 8
#     buffer_buffer = []
#     buffer_buffer_lim = 4
#
#
#     def write_sql(dfs, pgsql_address='localhost:5432', tablename="tn_core", schema="public"):
#         engine = create_engine(f"postgresql+psycopg2://plaggm@{db}/public")
#         engine.dispose()
#         with engine.connect() as conn:
#             for df in dfs:
#                 df.to_sql(tablename, con=conn, schema=schema, if_exists="append")
#         return dfs
#
#     def gen_bag():
#         x = db.from_sequence(buffer,npartitions=4).map(json_reader).filter(bag_filter)
#         x = x.map_partitions(write_sql)
#         x = client.persist(x)
#         return x
#
#     for l in tqdm.tqdm(nfg):
#         buffer.append(l)
#         if len(buffer) > bufferlim:
#             mx = gen_bag()
#             buffer_buffer.append(mx)
#             buffer = []
#         if len(buffer_buffer) > buffer_buffer_lim:
#             for m in tqdm.tqdm(buffer_buffer, desc="Queue up dask work"):
#                 #x = client.persist(m)
#                 bags.append(m)
#             buffer_buffer = []
#
#     if len(buffer_buffer) > 0:
#         for b in tqdm.tqdm(buffer_buffer, desc="emptying buffers"):
#             #x = client.persist(b)
#             bags.append(b)
#     if len(buffer) > 0:
#         bag = gen_bag()
#         #x = client.persist(bag)
#         bags.append(bag)
#     comps = []
#     #print(bags[0].compute())
#     click.secho(f"Num bags: {len(bags)}",fg='green')
#     bag_split = 32
#     bgs = []
#     ogb = []
#     for x in tqdm.tqdm(bags,desc ="writing to db"):
#         # x = db.concat(x).repartition(40)
#         # z = x.map_partitions(write_sql)
#
#         z = client.compute(x)
#         bgs.append(z)
#         if len(bgs) > 16:
#             # for bbx in tqdm.tqdm(as_completed(bgs),desc = "Waiting on BAGs" ):
#             #     pass
#             ogb = ogb + bgs
#             bgs = []
#
#     large_df = db.concat(bags)
#     print(large_df)
#     large_df = large_df.to_dataframe()
#     import os
#     out_file = f"{os.getcwd()}/fixed_{nfg.name}"
#     click.secho(f"out file: {out_file}",fg="green")
#     save_fix =client.submit(dd.to_json,large_df,out_file,orient="index")
#     click.secho("WORKING")
#     mdf = save_fix
#     #mdf = client.persist(save_fix)
#     import dask.distributed
#     print("PROG1")
#     dask.distributed.progress(mdf)
#     print("PROG2")
#     print(dask.distributed.progress(mdf))
#     print(mdf.compute())
#     dask.distributed.progress(z)
#     # for x in tqdm.tqdm(as_completed(bgs)):
#     #     pass
#     print(x.result())
#
#         #bgs.append(x.map_partitions(write_sql))
#         #if len(bgs) >= bag_split:
#         #    tqdm.tqdm.write("Writing out SQL")
#         #    big_bag = db.concat(bgs)
#         #    z = big_bag.compute()
#
#
#
#
#
#
#     # for ftr in tqdm.tqdm(as_completed(comps), desc="waiting on dask"):
#     #     pass
#     # print(ftr.result())
#
#     # mx = db.read_text(str(nfg.name)).repartition(multiprocessing.cpu_count()).map(json_reader).filter(bag_filter)
#     # mx = client.submit(mx)
#     # click.secho("Persisting on MX", fg="green")
#     # dx = []
#     # mx = client.persist(mx)
#     from distributed.diagnostics.progressbar import progress
#     # print(mx.take(2))
#
#     print("Table Write Wait")
#
#     # import psycopg2
#     # import sqlalchemy
#     # conn = psycopg2.connect(pgsql_address,)
#     # cur = conn.cursor()
#     # query = """INSERT INTO tn_core
#     # (core_uid,core_type,core_id,local_id,synaptic_connectivity,g_i,sigma_g,s,b,epsilon,sigma_lambda,lambda,c,alpha,beta,tm,vr,sigma_vr,gamma,kappa,signal_delay ,dest_core,dest_local,output_neuron,self_firing) VALUES(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)"""
#
#     # mx = client.persist(mx.to_dataframe())
#     # mx = dd.concat(bags)
#     # crx = client.submit(dd.to_csv,mx,"converted.csv",single_file=True)
#
#     # with tqdm.tqdm() as pbar:
#     #     done = False
#     #     while not done:
#     #         try:
#     #             crx.result(timeout=5)
#     #             done = True
#     #             click.secho("Done...", fg="green")
#     #         except:
#     #             pbar.write(f"Status: {crx.status}\h{client}")
#     #             pbar.update(1)
#     #
#     # return "converted.csv"
#
#
# @click.group()
# def cli():
#     click.clear()
#
#
# @cli.command()
# @click.argument("src_db", type=click.Path(exists=True, file_okay=True, dir_okay=False))
# @click.argument("dest_js", type=click.Path(exists=False, dir_okay=False))
# def genspikejs(src_db, dest_js):
#     click.secho(f"Reading spikes from {src_db}", fg="green")
#     df = pd.read_sql_table("input_spikes", src_db)
#     click.secho(f"Writing spikes to {dest_js}", fg="green")
#     df.to_json(dest_js, orient='records')
#
#
# @cli.command()
# @click.option('--dest', '-d', help="Destination sqlite file", show_default=True, default="converted.sqlite")
# @click.option('--dask', is_flag=True, help="Enable dask CSV", default=False)
# @click.argument('nfg', type=click.Path(file_okay=True, dir_okay=False, exists=True))
# @click.argument('spike_db', type=click.Path(file_okay=True, dir_okay=False, exists=True))
# def gensql(dest, dask, nfg, spike_db):
#     nfg = open(nfg, 'r')
#     click.secho("Convert NFG to sqlite data")
#     global_settings = get_globals(nfg)
#     click.echo(f"{global_settings}")
#     click.secho(f"Selected {nfg.name} ", fg="blue")
#     if dask:
#         daskmode(dest, nfg)
#     else:
#         convert_merge(dest, nfg, spike_db)
#
#
# if __name__ == '__main__':
#     cli()
