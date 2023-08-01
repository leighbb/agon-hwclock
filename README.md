# Agon MOS version of hwclock

## About

This project builds a single utility called hwclock.bin

It can be used to set and read the RTC on the
[Olimex MOD-RTC](https://www.olimex.com/Products/Modules/Time/MOD-RTC/open-source-hardware)
or the [Olimex MOD-RTC2](https://www.olimex.com/Products/Modules/Time/MOD-RTC2/open-source-hardware).

## Installation

Copy `hwclock.bin` into the MOS directory on your Agon MOS MicroSD card.

## Usage

    hwclock [ -debug ] [ -1 | -2 ] <command>

or

    hwclock -help

Options:

    -debug   Enable RTC debugging

    -1       Select MOD-RTC
    -2       Select MOD-RTC2

Commands:

    -systohc Set the System Clock from the Hardware Clock
    -hctosys Set the Hardware Clock from the System Clock

    -showhc  Show the date and time of the Hardware Clock
    -showsys Show the date and time of the System Clock

    -sethc   Set the Hardware Clock
    -setsys  set the System Time

## Examples

1. Set the MOD-RTC module to the given date and time

    `hwclock -1 -sethc 2022-04-07T08:30:00`

2. Show the current date and time of the MOD-RTC2 module

    `hwclock -2 -showhc`

3. Set the system clock from the MOD-RTC module

    `hwclock -1 -hctosys`

The above example can be placed in your `autoexec.txt` to automatically set
the system clock every time you switch on your Agon Light.

## Feedback

Raise an issue if you would like any additional features.

