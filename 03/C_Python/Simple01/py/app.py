#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os
import sys

print(sys.version)

def test(*args, **kwargs):
    print(args)
    print(kwargs)
    context = args[1]
    context['i'] = 222
    context['j'] = 599
    context['name'] = 'ZhangSan'