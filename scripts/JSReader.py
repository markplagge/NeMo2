import demjson
import multiprocessing
import pandas as pd
import re
import tqdm
from numba import jit


@jit
def add_q(match):
    z = str(match.group(0))
    return z


@jit
def rt(match):
    r = match.group()
    r = r.replace('{', '[').replace('}', ']')
    return r


class JSReader(multiprocessing.Process):
    def __init__(self, task_queue, result_queue, nid, db_path, table_name, do_sql=True, use_pandas=True):
        multiprocessing.Process.__init__(self)
        self.do_sql = do_sql
        self.table_name = table_name
        self.db_path = db_path
        self.task_queue = task_queue
        self.result_queue = result_queue
        self.neu_rem = re.compile(r"(neurons.\=\s?)")
        self.end_clean_comma = re.compile(r",$")
        self.end_clean_curl = re.compile(r"}[\s]+(})$")
        self.fix_tn = re.compile(r"TN_\d+_\d+")
        self.fix_labs = re.compile(r"\w*(?=\s\=)")
        self.fix_arrs = re.compile(r"\s({).?[\d,-]+\s(})")
        self.nid = nid
        self.sv = 0
        self.use_pandas = use_pandas
        self.lines_parsed_as_dict = True

    @jit
    def parse_json_line(self, line):

        remove_nrn = line.replace('neurons = ', '')
        remove_nre = re.sub(self.end_clean_comma, "", remove_nrn)
        remove_end = re.sub(self.end_clean_curl, "}", remove_nre)
        add_quotes = re.sub(self.fix_labs, add_q, remove_end)
        fin_quotes = add_quotes.replace("=", ":")
        fixed_ar = re.sub(self.fix_arrs, rt, fin_quotes)
        fixed_ar = fixed_ar + "}"
        fixed_ar = fixed_ar.lstrip()
        if not fixed_ar.startswith("{"):
            fixed_ar = "{" + fixed_ar

        if self.lines_parsed_as_dict:
            fixed_ar = demjson.decode(fixed_ar)

        return fixed_ar

    def parse_json_lines(self, lines):
        jsons = []
        for l in tqdm.tqdm(lines, position=self.nid + 1, desc=f"Process {self.nid} json lines"):
            j = self.parse_json_line(l)
            if not isinstance(j, dict) and self.lines_parsed_as_dict:
                j = demjson.decode(j)
            jsons.append(j)
        return jsons

    def parselogic(self, next_task):
        rr = []
        if isinstance(next_task, list):
            jp = self.parse_json_lines(next_task)
            rr = jp
        else:
            jp = self.parse_json_line(next_task)
            if self.lines_parsed_as_dict:
                jp = demjson.decode(jp)
            rr.append(jp)
        if self.use_pandas:
            rr = pd.DataFrame.from_dict(rr, orient='index')

        self.result_queue.put(rr)

    def write_sql(self, result_df):
        pass

    def run(self, ip=True):
        proc_name = self.name
        with tqdm.tqdm(position=self.nid, desc=f"Process {self.nid} working...") as pbar:
            while True:
                next_task = self.task_queue.get()
                if next_task is None:
                    print(f"JSON Parser done")
                    self.task_queue.task_done()
                    break
                else:
                    self.parselogic(next_task)
                pbar.update(1)
                self.task_queue.task_done()
                if ip == False:
                    break
