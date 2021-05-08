#!/bin/bash

sed '1 s:Ref:Designator:;1 s:PosX:Mid X:;1 s:PosY:Mid Y:;1 s:Rot:Rotation:;1 s:Side:Layer:' "$1"
