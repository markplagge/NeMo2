//
// Created by Mark Plagge on 5/2/20.
//

#ifndef NEMOTNG_MODELFILE_H
#define NEMOTNG_MODELFILE_H

#include <string>

class ModelFile {
 public:
  static ModelFile *Instance ();
  bool open_config_file (std::string logfile);
  int read_config_file ();
  bool close_config_file ();

 private:
  ModelFile () = default;
  ;
  ModelFile (ModelFile const &) = default;
  ;
  //ModelFile& operater = (ModelFile const&) {};
  static ModelFile *m_Instance;
};

#endif//NEMOTNG_MODELFILE_H
