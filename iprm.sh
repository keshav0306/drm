#!/bin/bash

for i in {131074..131076}
do
	ipcrm -m $i
done
