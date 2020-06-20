//
// Created by Mark Plagge on 5/2/20.
//

#include "NemoCoreLPWrapper.h"
#include "../../nemo_config/NemoConfig.h"
#include "../../nemo_io/ModelFile.h"
void nemo::neuro_system::NemoCoreLPWrapper::set_core_id (NemoCoreLPWrapper *s, tw_lp *lp)
{
  auto global_id = lp->gid;
  s->core_id = get_core_id_from_gid (global_id);
}
