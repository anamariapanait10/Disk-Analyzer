# Disk Analyzer Daemon

### Project made by:
    - Neaga-Budoiu Maria
    - Panait Ana-Maria
    - Teodorescu George-Tiberiu 
    - Trufas Dafina

## Project requirement:
- Create a daemon that analyzes the space used on a storage device starting from a given path, and build a utility program that allows using this functionality from the command line.
- The daemon must analyze the occupied space recursively, for each directory content, regardless of the depth.

## Setup (only the first time):

-> Create a service called `disk-analyzer.service`in `/etc/systemd/system` with the following content:

    [Unit]
    Description=disk-analyzer

    [Service]
    User=your_user
    Group=your_user
    ExecStart=/path/to/daemon
    Restart=always

    [Install]
    WantedBy=multi-user.target

-> Open `/etc/sudoers` and write `your_user ALL=(ALL:ALL) NOPASSWD: /path/to/daemon` \
-> Run `systemctl daemon-reload` command. \
-> Run `systemctl enable disk-analyzer.service` command. \
-> Run `systemctl start disk-analyzer.service` command. \
-> Run `init.c` script that adds `da` script to the PATH variable.

## Help message:
```
Usage: da [OPTION]... [DIR]... 
Analyze the space occupied by the directory ai [DIR] 
    -a, --add           analyze the new directory path for disk usage
    -p, --priority      set priority for the new analysis (works only with
                        -a argument)
    -S, --suspend <id>  task with id <id>
    -R, --resume <id>   resume task with <id>
    -r, --remove <id>   remove the analysis with the given <id>
    -i, --info <id>     print status about the analysis with                
                        <id> (pending, progress, complete)
    -l, --list          list all analysis tasks, with their ID and
                        the corresponding root priority
    -p, --print <id>    print analysis report for those tasks that are 'done'
```

## Usage example:
```
$> da -a /home/user/my_repo -p 3
Created analysis task with ID ’2’ for ’/home/user/my_repo’ and priority ’high’.

$> da -l
ID  PRI     Path               Done Status               Details
2   ***  /home/user/my_repo  45% in progress       2306 files, 11 dirs

$> da -l
ID PRI       Path              Done Status               Details
2  ***  /home/user/my_repo   75% in progress       3201 files, 15 dirs
     
$> da -p 2
Path               Usage  Size                Amount
/home/user/my_repo 100%   100MB  #########################################
|
|-/repo1/ 31.3%  31.3MB #############
|-/repo1/binaries/ 15.3%  15.3MB ######
|-/repo1/src/ 5.7%   5.7MB ##
|-/repo1/releases/ 9.0%   9.0MB  ####
|
|-/repo2/ 52.5%  52.5MB #####################
|-/repo2/binaries/ 45.4%  45.4MB ##################
|-/repo2/src/ 5.4%   5.4MB ##
|-/repo2/releases/ 2.2%   2.2MB #
|
|-/repo3/ 17.2%  17.2MB ########
[...]
    
$> da -a /home/user/my_repo/repo2
Directory ’home/user/my_repo/repo2’ is already included in analysis with ID ’2’

$> da -r 2
Removed analysis task with ID ’2’, status ’done’ for ’/home/user/my_repo’

$> da -p 2
No existing analysis for task ID ’2’
```


