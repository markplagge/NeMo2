//
// Created by Mark Plagge on 5/2/20.
//

#include "ModelFile.h"
// Global static pointer used to ensure a single instance of the class.

ModelFile *ModelFile::m_Instance = NULL;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
ModelFile *ModelFile::Instance ()
{
  if (!m_Instance)// Only allow one instance of class to be generated.
	m_Instance = new ModelFile;

  return m_Instance;
}

bool ModelFile::open_config_file (std::string logfile)
{
  return false;
}

int ModelFile::read_config_file ()
{
  return 0;
}

bool ModelFile::close_config_file ()
{
}