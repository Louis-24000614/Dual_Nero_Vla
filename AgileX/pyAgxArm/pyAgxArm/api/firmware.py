#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import re
from typing import Tuple

from .arm_options import ArmModel, NeroFW


_NERO_VERSION_PATTERN = re.compile(r"(\d+)\.(\d{2,})")


def _parse_nero_version(firmware_version: str, robot: str) -> Tuple[int, int, int]:
    if not isinstance(firmware_version, str):
        raise TypeError("firmware_version must be a string")

    match = _NERO_VERSION_PATTERN.fullmatch(firmware_version.strip())
    if match is None:
        raise ValueError(
            "Invalid firmware version {!r} for robot {!r}".format(
                firmware_version, robot
            )
        )

    compact_minor = match.group(2)
    return (
        int(match.group(1)),
        int(compact_minor[:2]),
        int(compact_minor[2:] or "0"),
    )


def resolve_firmware_profile(robot: str, firmware_version: str) -> str:
    """Resolve a Nero firmware version to an SDK driver profile.

    Parameters
    ----------
    robot : str
        Robot model. Use ``ArmModel.NERO``.

    firmware_version : str
        Version returned by ``get_firmware()``, such as ``"1.20"``.

    Returns
    -------
    str
        Corresponding ``NeroFW`` profile. The result can be passed to
        ``create_agx_arm_config(..., firmeware_version=...)``.

    Raises
    ------
    ValueError
        If ``robot`` is unsupported or the firmware version does not use the
        expected format.

    TypeError
        If ``firmware_version`` is not a string.

    Examples
    --------
    >>> from pyAgxArm import ArmModel, resolve_firmware_profile
    >>> resolve_firmware_profile(ArmModel.NERO, "1.20")
    'v120'
    """
    if robot == ArmModel.NERO:
        version = _parse_nero_version(firmware_version, robot)
        if version >= (1, 21, 0):
            return NeroFW.V121
        if version >= (1, 20, 0):
            return NeroFW.V120
        if version >= (1, 12, 0):
            return NeroFW.V112
        if version >= (1, 11, 0):
            return NeroFW.V111
        return NeroFW.DEFAULT

    raise ValueError("Unsupported robot model: {!r}".format(robot))
