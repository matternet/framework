#!/usr/bin/env python3

import pyuavcan_v0 as uavcan
import argparse
import os
import em   # NOTE: must use empy v3.3.4
from canard_dsdlc_helpers import *

templates = [
    {'source_file': 'request.h',
     'output_file': 'include/@(msg_header_name_request(msg))'},
    {'source_file': 'response.h',
     'output_file': 'include/@(msg_header_name_response(msg))'},
    {'source_file': 'broadcast.h',
     'output_file': 'include/@(msg_header_name(msg))'},
    {'source_file': 'service.h',
     'output_file': 'include/@(msg_header_name(msg))'},
    {'source_file': 'request.c',
     'output_file': 'src/@(msg_c_file_name_request(msg))'},
    {'source_file': 'response.c',
     'output_file': 'src/@(msg_c_file_name_response(msg))'},
    {'source_file': 'broadcast.c',
     'output_file': 'src/@(msg_c_file_name(msg))'},
]

parser = argparse.ArgumentParser()
parser.add_argument('namespace_dir', nargs='+')
parser.add_argument('build_dir', nargs=1)
parser.add_argument('--build', action='append')
args = parser.parse_args()

buildlist = None

if args.build:
    buildlist = set(args.build)

namespace_paths = [os.path.abspath(path) for path in args.namespace_dir]
build_dir = os.path.abspath(args.build_dir[0])

os.chdir(os.path.dirname(__file__))
templates_dir = 'templates'

messages = uavcan.dsdl.parse_namespaces(namespace_paths)
message_dict = {}
for msg in messages:
    message_dict[msg.full_name] = msg

for template in templates:
    with open(os.path.join(templates_dir, template['source_file']), 'rt') as f:
        template['source'] = f.read()

def build_message(msg_name):
    print('building %s' % (msg_name,))
    msg = message_dict[msg_name]
    for template in templates:
        output = em.expand(template['source'], msg=msg)

        if not output.strip():
            continue

        output_file = os.path.join(build_dir, em.expand('@{from canard_dsdlc_helpers import *}'+template['output_file'], msg=msg))
        mkdir_p(os.path.dirname(output_file))
        with open(output_file, 'wt') as f:
            f.write(output)

if __name__ == '__main__':
    if buildlist is not None:
        while True:
            new_buildlist = set(buildlist)
            for msg_name in buildlist:
                msg = message_dict[msg_name]
                fields = getattr(msg, 'fields', []) + getattr(msg, 'request_fields', []) + getattr(msg, 'response_fields', [])
                for field in fields:
                    if field.type.category == field.type.CATEGORY_COMPOUND:
                        new_buildlist.add(field.type.full_name)

            if not new_buildlist-buildlist:
                break

            buildlist = new_buildlist

    from multiprocessing import Pool

    pool = Pool()
    builtlist = set()
    if buildlist is not None:
        for msg_name in buildlist:
            builtlist.add(msg_name)
            pool.apply_async(build_message, (msg_name,))
    else:
        for msg_name in [msg.full_name for msg in messages]:
            print('building %s' % (msg_name,))
            builtlist.add(msg_name)
            pool.apply_async(build_message, (msg_name,))

    pool.close()
    pool.join()

    assert buildlist is None or not buildlist-builtlist, "%s not built" % (buildlist-builtlist,)
