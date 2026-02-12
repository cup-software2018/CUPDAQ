R__LOAD_LIBRARY(libDAQConfig)
R__LOAD_LIBRARY(libyaml-cpp)

void readconfig()
{
  RunConfig run;
  //run.SetConfigFilename("fadct.config");

  run.ReadConfig("fadct.config");

  AbsConfList * configs = run.GetConfigs();
  configs->Dump();
}