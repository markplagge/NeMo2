import os

#!/bin/python3
from pathlib import Path
import argparse
import logging
import subprocess as sp
import os

codes_env_args = {
    "BISON":"",
    "CC_COMPILER":"",
    "CXX_COMPILER":"",
    "PKG_CONFIG_PATH":"",
    "CFLAGS":"",
    "CXXFLAGS":""}

def append_path(new_path_value = "/usr/bin"):
    old_path = os.environ.get("PATH")
    print(old_path)
    new_path = new_path_value + ":" + old_path
    return new_path

def append_path_entries(new_path_value_list = []):
    old_path = os.environ.get("PATH")
    for path_entry in new_path_value_list:
        old_path =  path_entry + ":" + old_path
    return old_path


def run_codes_config(autoreconf = None, codes_path=None,
                     config_args=None, env_parms=None, path_over=None):
    if autoreconf is None:
        autoreconf = "autoreconf"
    if codes_path is None:
        codes_path = "/Users/plaggm/CLionProjects/untitled/external/codes-src"
    if path_over is None:
        path_over = ["/usr/local/share/aclocal","/usr/local/opt/bison/bin"]
    if env_parms is None:
        env_parms = {}
    if config_args is None:
        config_args = {"prefix": "/Users/plaggm/CLionProjects/untitled/cmake-build-debug/codes-bin"}
    logging.debug(f"Running CODES config in path {codes_path}.\n Config: {config_args} \n ENV: {env_parms}")
    path = os.environ.get("PATH")
    env = os.environ.copy()
    if len(path_over) > 0:
        path = append_path_entries(path_over)
    env['PATH'] = path
    for ev_name, ev_val in env_parms.items():
        env[ev_name] = ev_val


    codes_base_config = [autoreconf ,  codes_path , "-fi", "-Im4"]
    codes_config_exe = [codes_path + "/configure"]
    #codes_config_arg = f"{' '.join(['--'+pname + '=' + pval for pname,pval in config_args.items()])}"
    codes_config_arg = ['--'+pname + '=' + pval for pname,pval in config_args.items()]
    codes_config = codes_config_exe + codes_config_arg
    print("-" * 100)
    print("-" * 20)
    print(codes_config)
    print("-" * 20)

    sp.run(["echo", "$PKG_CONFIG_PATH"], env=env, shell=True)
    sp.run(["echo","$PATH"],env=env,shell=True)
    print("-" * 100)

    sp.run(codes_base_config,shell=True,cwd=codes_path)
    sp.run(codes_config,env=env)



def split_dict(arg, pair_delim=":",kv_delim=":"):
    print(arg)
    result_dict = {}
    k,v = arg.split(kv_delim)
    result_dict[k] = v
    return result_dict

def split_dict_pairs(arg, pair_delim=",", kv_delim=":"):
    print(arg)
    result_dict = {}
    for item in arg.split(pair_delim):
        k,v = item.split(kv_delim)
        k = k.lstrip()
        result_dict[k] = v
    return result_dict

def base_path(arg):
    print(arg)
    cp = Path(arg)
    if cp.exists() and cp.is_dir():

        return str(cp.absolute())
    else:
        raise argparse.ArgumentTypeError(arg + " is not a vaild / existing path.")

def list_type(arg, delim=","):
    v = []
    for i in arg.split(delim):
        v.append(i)
    return v

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--autoreconf_path",type=str)
    parser.add_argument("--paths","-p",type=list_type,help="Path entry overrides for the codes config env. In the "
                                                           "form of 'ENTRY_1,ENTRY_2'")
    parser.add_argument("--env_vars","-e", type=split_dict_pairs,help="env. variables to pass to CODES config. Form of 'ENV_VAR_NAME:VALUE,ENV_VAR_2:VALUE2'")
    parser.add_argument("--codes_path", "-c", type=base_path)
    parser.add_argument("--autoconfig_opts","-a", type=split_dict_pairs, help="Config options for CODES. Form of "
                                                                              "'arg_name:val,arg_name:val")
    parser.add_argument("--flat_namespace", type=bool, help="Add flat namespace")

    print(os.getcwd())
    args = parser.parse_args()

    print(args)


    #if args.CMAKE_VALE is None:
    run_codes_config(args.autoreconf_path,args.codes_path,args.autoconfig_opts, args.env_vars,args.paths)
    #else:
    #    logging.info("CMAKE configure of CODES")







