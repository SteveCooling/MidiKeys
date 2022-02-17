#!/usr/bin/env python3

import math

velocity_factor = 0.2
velocity_curve  = 60


# max human speed about 3ms
# slowest sensible speed around 64 ms
#
# we want near perfect lookups with a log curve

def midiFreq(note):
    return int(440*math.pow(2,(note-69)/12))

def outputCArray(name, length, callback):
    print("static const int {}[{}] = {{".format(name, length))
    for i in range(length):
        print("  {},".format(callback(i)))
    print("};")


print(
'''/*
*
*  THIS FILE IS AUTO GENERATED
*
*  Generated by tools/genlookup.py - do not modify this filedirectly , it might be overwritten
*
*/
'''
)

outputCArray("midi_freq", 128, midiFreq)

