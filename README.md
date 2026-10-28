# thorfinn
desktop pipelining tool written in c++

```
****************************************************
*********************##*+==+************************
******************+==+-==-=-----********************
******************-**=--*-**-=----=+****************
**++++*********==++=--+-=*=-==*-#--=-*+++***********
*****+=********++++#=+-+*+=+*==+=++=--++=+***+=++++*
**+++++++++*****+++=+===+====*=+=*+=+-=+=-+*++-=-=+*
***:--===++*+=+*++=+*+==*====*=+==**+=*+**+=:-=++*++
=++-::::-=**++**++==+++=*==+*++*=+====+++++---==++++
--=----:::-=+**%****=#*++*+=***%*+==*=#++===++-=++++
=++++==+==--*+=*#**#**%%#*#+*#%=%+++*+**=+++++++++++
++++++++++++++#+*#**#---::===----=****++++++++++++++
++++++++++++++##+#***+-::::::---*%**##**++++++++++==
++++++++++++++**#***#+--:-++----**##*%#++++==+=--=--
=++++++++++++*%@#%**%%*---==---#%*%#%%@@*+==--=:::=.
=+++++++++++++#@@%#*%%*#*-:--*#*%%#%##**=+++=-=-:.:-
-==-==========***++#########%####+===+***====----+=-
::::-========***#+======+####+===+==+#****---:===+=+
::::::-====****+*##=-==***%%#***==+##+=++=+*+=+===+-
::-*****#***+=====+*###***%%**###******+====+**==-+*
:*++::+#***+========++***###****#***==========*+====
=*==:-#****====+**==++===#*#*++*#*===++====--==*+=++
+*-+=*****==+**+===++++==+##====++=======+*++++**:-+
*+:-=#*****+=======++++==+**+*++++===========*#**=:=
=*=-****===========++++==+##===++*============****+:
-*%*#**+===========++=+++*##+=+++=======++=====***+-
#*+#***+==============++=##%#+=============+*#*****#
%#%%%%%#*+===+=========+*#####+=====+==========+#%%%

```

## building (dev & quick testing)
- run the provided `build.sh` file. 
- make sure you have `yaml-cpp` installed.
- if the provided build file doesn't work for you, check the given lib paths.

## building (prod)
- make sure you have `cmake` installed on your system
- create a build directory using `mkdir build` 
- navigate into that directory using `ls build`
- run `cmake ..`
- after the process has finished, run `make`

## usage
- `./thorfinn make`: to prepare a pipeline in your current directory.
- `./thorfinn exec <?path>`: executes the pipeline in given / current directory. path argument is optional
- `./thorfinn listen <?path>`: listens for defined events to trigger pipeline execution in the given / current directory. path argument is optional

### trigger types
- `manual`: only manual execution, when directly ran through `exec`.
- `on_event[event-type]`: event-based execution. The following event types are currently supported:
    - `file_change`: triggers when a specified file is modified. Configuration requires a `path` key.
    - `interval`: triggers the pipeline at a specified interval. Configuration requires a `seconds` key.
    - `webhook`: [not fully implemented] triggers when a webhook is received on a specific endpoint.
- `automatic[cron]`: [not fully implemented] cron-based execution.

### ssh authentication
- `ssh_global_config` and the `establish_ssh` action accept `host`, `port`, `username` and either:
    - `private_key` (path, `~` allowed), optional `public_key` and `passphrase`: public key authentication, or
    - `password`: password authentication, used only when no `private_key` is set.

## disclaimer
code should not be used in production, just a learning project for myself blah blah blah you know how it goes
