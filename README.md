# nce - A cheat engine clone for Linux

This program helps you cheat at games by directly modifying memory values in another process.

## Usage

nce needs a "scratch file", which is really just a list of memory addresses. Basic usage looks
like this:

    nce [other process pid] [command]

There are three commands:

    search [needle] [scratch file]         Searches a process for matches and stores them
    prune [needle] [scratch file]          Removes addresses that don't match the new needle
    set [value] [scratch file]             Updates all the addresses in the scratch file to a new value

Values (the stuff you put in for needle/value) look like one of the following:

    b12    - 12 as a byte
    w988   - 988 as a two byte word
    l721   - 721 as a four byte long
    q123   - 123 as an eight byte quadword
    shello - hello as a null-terminated string

I wrote this program around a year ago, I don't know why I didn't put signed ints here, that
would have been so easy to implement.

I do remember why i didn't put floats or doubles here, that would have required some imprecision,
which would have been annoying to implement for thi simple proof of concept.
