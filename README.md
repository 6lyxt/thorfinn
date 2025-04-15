# thorfinn
desktop pipelining tool written in c++

## building
run the provided `build.sh` file. 
make sure you have `yaml-cpp` installed.
if the provided build file doesn't work for you, check the given lib paths.

## usage
`./thorfinn make`: to prepare a pipeline in your current directory.
`./thorfinn exec <?path>`: executes the pipeline in given / current directory. path argument is optional

### pipeline types
`manual`: only manual execution, when directly ran through `exec`.
[not fully implemented] `event[event-type]`: event based execution, for example "onMouseClick". a full list of available events will be published soon.
[not fully implemented] `automatic[cron]`: cron based execution

## disclaimer
code should not be used in production, just a learning project for myself blah blah blah you know how it goes
