"""Mapping body IDs to robot numbers."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import memory

CONFIG_MAP = {'ALDR1109F002819': 35,
              'ALDR1109F003025': 36,
              'ALDR1109F006024': 40,
              'ALDR1109F005848': 41,
              'ALDR1109F005830': 42,
              'ALDR1109F006058': 43,
              'ALDR1109F005868': 44,
              'ALDR1109F005869': 45,
              'ALDR1109F005866': 46,
              'ALDR1109F005825': 47,
              'ALDR1312N090321': 50,
              'ALDR1312N090229': 51,
              'ALDR1312N090241': 52,
              'ALDR1312N090206': 53,
              'ALDR1312N090368': 54,
              'ALDR1312N090218': 55,
              }


def get_config_id():
    """Map body id to serial number."""
    bid = memory.robot_state.bodyId()
    if bid in CONFIG_MAP:
        cid = CONFIG_MAP[bid]
    else:
        cid = memory.robot_state.robot_id_
    print('Loaded config_id=%i for body_id=%s (%s)' % (
      cid,
      memory.robot_state.bodyId(),
      ("success" if bid in CONFIG_MAP else "BODY ID MISSING")
    ))
    return cid
