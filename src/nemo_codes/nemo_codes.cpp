#include "../../include/nemo_codes/nemo_codes.h"

#include "../nemo/nemo_config/NemoConfig.h"
#include <iostream>
#include <nemo.h>

#include "codes/model-net.h"
#include "codes/lp-io.h"
#include "codes/codes.h"
#include "codes/codes_mapping.h"
#include "codes/configuration.h"
#include "codes/lp-type-lookup.h"

/** @defgroup primary_nemo_interface
 * Primary event handlers for NeMo - TNG.
 *  Currently these exist here to help simplify the code.
 *  @TODO: Will move these into the CoreLP (CoreLP.h) system to keep things consistant.
 *  @{
 */

void
nemo_init (INeuroCoreBase *core, tw_lp *lp)
{
  core->core_init (lp);
}
void
nemo_pre_run (INeuroCoreBase *core, tw_lp *lp)
{
  core->pre_run (lp);
}
void
nemo_forward_event (INeuroCoreBase *core, tw_bf *bf, nemo_message *m, tw_lp *lp)
{
  core->forward_event (bf, m, lp);
}

void
nemo_reverse_event (INeuroCoreBase *core, tw_bf *bf, nemo_message *m, tw_lp *lp)
{
  core->reverse_event (bf, m, lp);
}
void
nemo_commit (INeuroCoreBase *core, tw_bf *bf, nemo_message *m, tw_lp *lp)
{
  core->core_commit (bf, m, lp);
}
void
nemo_finish (INeuroCoreBase *core, tw_lp *lp)
{
  core->core_finish (lp);
}


// lp reg:

tw_lptype nemo_lp = {
	(init_f)nemo_init,
	(pre_run_f)nemo_pre_run,
	(event_f)nemo_forward_event,
	(revent_f)nemo_reverse_event,
	(commit_f)nemo_commit,
	(final_f)nemo_finish,
	(map_f)codes_mapping,
	sizeof (INeuroCoreBase),
};

tw_lptype lif_core = {
	(init_f)nemo_init,
	(pre_run_f)nemo_pre_run,
	(event_f)nemo_forward_event,
	(revent_f)nemo_reverse_event,
	(commit_f)nemo_commit,
	(final_f)nemo_finish,
	(map_f)codes_mapping,
	sizeof (LIFCore),
};

void
nemo_add_lp_type ()
{
  lp_type_register ("")
}

/** @} */


int
main (int argc, char **argv)
{

  int nprocs;
  int rank;
  int num_nets;
  int *net_ids;
  int num_router_rows, num_router_cols;

  tw_opt_add (nemo_tw_options);

  tw_init (&argc, &argv);

  if (argc < 2)
	{
	  printf ("\n Usage: mpirun <args> --sync=2/3 mapping_file_name.conf (optional --nkp) \n");
	  MPI_Finalize ();
	  return 1;
	}

  MPI_Comm_rank (MPI_COMM_CODES, &rank);
  MPI_Comm_size (MPI_COMM_CODES, &nprocs);

  configuration_load (argv[2], MPI_COMM_CODES, &config);

  return 0;

}