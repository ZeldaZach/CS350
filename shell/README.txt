Created by Zach Halpern for CS350 Fall 2017

This project revolves around a 'mini' shell script that will have some basic features of the main shell.

To start:
`$ ./minish`

It will accept any normal command found in the $PATH and execute as it should.
Examples:
`$ ls`

`$ ls -lah`

`$ sleep 30`

It can also throw processes into the background
Example:
`$ sleep 40 &`

There are custom implementations of the `cd`, `pwd`, and `listjobs` commands.

listjobs will list all background processes with their current status.

`fg <PID>` should pull a process from the background to the foreground. It will wait for it to properly finish.

If an interrupt (SIGINT) is found, the process will catch it and continue.
