# <p align="center"><img src="/tex/2b0e90db259cb83be63c505be10a2daf.svg?invert_in_darkmode&sanitize=true" align=middle width=146.3428329pt height=27.941632949999995pt/></p>

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/75fb4b9636a14f8bb220f03480d541df)](https://www.codacy.com/manual/plaggm/NeMo2?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=markplagge/NeMo2&amp;utm_campaign=Badge_Grade)
[![Code-Inspect](https://www.code-inspector.com/project/10414/score/svg)][code-inspect]
[![BCH compliance](https://bettercodehub.com/edge/badge/markplagge/NeMo2?branch=master)](https://bettercodehub.com/)
[![CodeFactor](https://www.codefactor.io/repository/github/markplagge/nemo2/badge)](https://www.codefactor.io/repository/github/markplagge/nemo2)
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![CodeScene Code Health](https://codescene.io/projects/8568/status-badges/code-health)](https://codescene.io/projects/8568)
----------------------------------------------------------------------------------------------------------------------------------

*NeMo* is a neuromorphic hardware simulation leveraging the extremely
fast and slim ROSS engine. Simulation of various neuromorphic hardware
is supported.

## Features

* Distributed simulation - Using MPI, *NeMo* will run on large-scale
  compute clusters
* Capable of simulating large scale hardware
  * Using ROSS, *NeMo* was able to scale a simulation of millions of
    neurons across over 65,000 processors
* **NEW**: New neuron models can be added to the simulation by
  subclassing the
  [base neuron model](src/nemo/nemo_neuro_system/neuron_models/NemoNeuronGeneric.h).
* Hardware configurations, models, and input spikes are generic JSON
  files.
* Preliminary neuromorphic process scheduling support


## Contents

- [Features](#features)
- [Requirements & Building](#requirements-and-building)
- [Usage and Examples](#usage-and-examples)
- [Configuration File Documentation](#configuration-file-documentation)
- [Neuromorphic OS Development](#neuromorphic-os-development)
- [Test Data](#test-data)
- [Development](#development)
- [References](#references)

## Requirements and Building

In order to build *NeMo* you will need:
* An installed MPI implementation for your system. OpenMPI and MPICH are
  good options. In general, any MPI compatible with [ROSS][ROSS] will
  work.
* A mostly C++ 14 compatible compiler
  * Tested with GCC <img src="/tex/d786805c2f14374b414f7a21b8b9348d.svg?invert_in_darkmode&sanitize=true" align=middle width=25.570741349999988pt height=21.18721440000001pt/> , Clang, and IBM XLC++ 14
* CMake <img src="/tex/a0336327bb0c1f07c69c5fb8f3f8be5e.svg?invert_in_darkmode&sanitize=true" align=middle width=46.57538489999998pt height=21.18721440000001pt/> - But current dev branch is using CMake 3.15

### Building

<img src="/tex/11668b75bf0fc55a0819686a86788993.svg?invert_in_darkmode&sanitize=true" align=middle width=62.38010459999999pt height=22.465723500000017pt/> uses CMAKE to build libraries and binaries (apps and
tests). Execute following commands to build *NeMo*&middot;2 using CMAKE:
1. `mkdir build`
2. `cd build`
3. `cmake -D CMAKE_BUILD_TYPE=Release ..`
4. `make nemo2` &#8592; makes the primary program.

Tests can be made using `make tests`.  
This will create binaries in `bin/` and `./` directory and libraries in
`lib/` directory. Tests are built under the `tests` folder. An example
configuration file is copied to the `<BUILD FOLDER>/config` from the
[example config folder](config/).

A future release will include a smaller test application!

More detailed install instructions are coming in the Wiki

## Usage and Examples

To run <img src="/tex/11668b75bf0fc55a0819686a86788993.svg?invert_in_darkmode&sanitize=true" align=middle width=62.38010459999999pt height=22.465723500000017pt/>:

#### As a Sequential Run (Not Distributed):

`./nemo2 --cfg:<PATH_TO_CONFIG_FILE.json>`

#### Distributed:

```
mpirun -np XX  ./nemo2 --cfg:<PATH_TO_CONFIG_FILE.JSON> --synch=[2|3|5] 
```

There are many configuration tweaks for the ROSS simulation as well as
other config options for *NeMo*&middot;2. To get a list of options run:

```
./nemo2 --help
```

Most of <img src="/tex/11668b75bf0fc55a0819686a86788993.svg?invert_in_darkmode&sanitize=true" align=middle width=62.38010459999999pt height=22.465723500000017pt/>'s options are handled by the configuration file.

## Configuration File Documentation

<img src="/tex/11668b75bf0fc55a0819686a86788993.svg?invert_in_darkmode&sanitize=true" align=middle width=62.38010459999999pt height=22.465723500000017pt/>'s configuration file is a standard JSON document.
Details will be coming in the Wiki. :shipit:

_note_: <img src="/tex/11668b75bf0fc55a0819686a86788993.svg?invert_in_darkmode&sanitize=true" align=middle width=62.38010459999999pt height=22.465723500000017pt/> expects a JSON standard that does not have line
comments. The examples below contain comments to help explain the
sections, but __do not__ use them in your config file.


Some helpful config options are:

JSON Setting | Details

| Setting                                     | Details                                                                              |
|:--------------------------------------------|:-------------------------------------------------------------------------------------|
| GLOBAL_DEBUG,                               | Global Debug - True prints out  a lot of info                                        |
| ns_cores_per_chip                           | How many neurosynaptic cores exist per chip?                                         |
| total_chips                                 | Total number of chips in the simulation                                              |
| do_neuro_os                                 | Activate neuromorphic OS                                                             |
| sched_type                                  | Scheduling algorithm                                                                 |
| save_nos_stats                              | Save neuromorphic operating system stats                                             |
| save_all_spikese                            | Save all output spike files                                                          |
| save_membrane_pots                          | Save per-event membrane potentials                                                   |
| neurons_per_core                            | Neurons per neurosynaptic core                                                       |
| output_spike_file                           | Where to save output spike files                                                     |
| output_nos_stat_file                        | Where to save neuromorphic OS files                                                  |
| output_membrane_pot_file                    | Where to save membrane potential record                                              |
|                                             | ***MODELS***                                                                         |
| id                                          | Model ID. Must be in sequence and unique (0,1...)                                    |
| needed_cores                                | NS cores needed by model. 0 indicates the model file will set this                   |
| model_file_path": "./models/cifar_100_model | Path to the JSON model file                                                          |
| spike_file_path": "./cifar_100_spike        | Path to the spike JSON (messagepack supported as well) file                          |
| requested_time,                             | Time requested for the model. 0 indicates forever (queue up all input spikes)        |
| benchmark_model_name":                      | If this is a benchmark model, specifiy it here. File paths are ignored in this case. |

Defaults for this file look like:
```json5
{
  "nemo_global": {
    "GLOBAL_DEBUG": true,
    "ns_cores_per_chip": 32,
    "total_chips": 2,
    "do_neuro_os": false,
    "sched_type": "FCFS",
    "save_nos_stats": true,
    "save_all_spikes": false,
    "save_membrane_pots": false,
    "neurons_per_core": 256,
    "output_spike_file": "./model_output.csv",
    "output_nos_stat_file" : "./output_nos_stat_file.csv",
    "output_membrane_pot_file": "./output_membrane_pot.csv",
    "core_type_ids": [
      0,
      1
    ]
  },
  "models":[
  "MODELS GO HERE"
  ]
  }
}
```


## Neuromorphic OS Development

Currently under development - Neuro OS features the ability to run spike
based neuron process scheduling. More to come.

## Test Data

A few demo models are available. We are finding a hosting provider to
store them, as a model file in <img src="/tex/11668b75bf0fc55a0819686a86788993.svg?invert_in_darkmode&sanitize=true" align=middle width=62.38010459999999pt height=22.465723500000017pt/> is quite a large file.

## Development

Under massive development.

### Current Work

## References

Check out the original *NeMo* [simulating many millions of
neurons][pads-paper] on a BlueGeneQ!

[codacy]: http://www.google.com
[code-inspect]:https://frontend.code-inspector.com/public/project/10414/neuro_os/dashboard "Code-inspector dashboard"
[pads-paper]: http://doi.acm.org/10.1145/2901378.2901392
[ROSS]: https://github.com/ross-org/ROSS "ROSS"

