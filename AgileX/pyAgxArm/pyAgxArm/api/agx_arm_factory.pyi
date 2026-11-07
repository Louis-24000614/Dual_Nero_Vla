# agx_arm_factory.pyi
from typing_extensions import Literal
from typing import Any, TypeVar, overload
from ..protocols.can_protocol.drivers import (
    NeroDriverDefault,
    NeroDriverV111,
    NeroDriverV112,
    NeroDriverV120,
    NeroDriverV121,
)


class NeroCanDefaultConfig():
    pass
class NeroCanV111Config():
    pass
class NeroCanV112Config():
    pass
class NeroCanV120Config():
    pass
class NeroCanV121Config():
    pass

# ---------- create_agx_arm_config overloads ----------

# --- nero ---

@overload
def create_agx_arm_config(
    robot: Literal["nero"],
    comm: Literal["can"] = ...,
    firmeware_version: Literal["default"] = ...,
    **kwargs: Any
) -> NeroCanDefaultConfig: ...

@overload
def create_agx_arm_config(
    robot: Literal["nero"],
    comm: Literal["can"] = ...,
    firmeware_version: Literal["v111"] = ...,
    **kwargs: Any
) -> NeroCanV111Config: ...

@overload
def create_agx_arm_config(
    robot: Literal["nero"],
    comm: Literal["can"] = ...,
    firmeware_version: Literal["v112"] = ...,
    **kwargs: Any
) -> NeroCanV112Config: ...

@overload
def create_agx_arm_config(
    robot: Literal["nero"],
    comm: Literal["can"] = ...,
    firmeware_version: Literal["v120"] = ...,
    **kwargs: Any
) -> NeroCanV120Config: ...

@overload
def create_agx_arm_config(
    robot: Literal["nero"],
    comm: Literal["can"] = ...,
    firmeware_version: Literal["v121"] = ...,
    **kwargs: Any
) -> NeroCanV121Config: ...

# ---------- AgxArmFactory ----------

T = TypeVar("T", bound=Any)

class AgxArmFactory:

    @classmethod
    @overload
    def create_arm(cls, config: None, **kwargs) -> None:
        """
        Create a robotic arm Driver instance.
        """
        ...

    # --- nero ---

    @classmethod
    @overload
    def create_arm(cls, config: NeroCanDefaultConfig, **kwargs) -> NeroDriverDefault:
        """Nero CAN driver (default, firmware <= v110).

        Terminology
        -----------
        `flange`:
        - The mounting face / connection interface on the robotic arm's last link
          (mechanical tool interface).

        Common conventions
        ------------------
        `timeout` (for request/response style APIs):
        - `timeout < 0.0` raises ValueError.
        - `timeout == 0.0`: non-blocking; evaluate readiness once and return
          immediately.
        - `timeout > 0.0`: blocking; poll until ready or timeout expires.

        `joint_index`:
        - `joint_index == 255` means "all joints".

        `set_*` return semantics:
        - Many `set_*` APIs are ACK-only: True means the controller acknowledged the
          request.
          This does not strictly guarantee the setting is already applied.
        - Some `set_*` APIs additionally verify by reading back state; their
          docstrings will mention the verification method if applicable.
        """
        ...

    @classmethod
    @overload
    def create_arm(cls, config: NeroCanV111Config, **kwargs) -> NeroDriverV111:
        """Nero CAN driver for firmware == v111."""
        ...

    @classmethod
    @overload
    def create_arm(cls, config: NeroCanV112Config, **kwargs) -> NeroDriverV112:
        """Nero CAN driver for firmware == 1.12."""
        ...

    @classmethod
    @overload
    def create_arm(cls, config: NeroCanV120Config, **kwargs) -> NeroDriverV120:
        """Nero CAN driver for firmware == v120 (1.20)."""
        ...

    @classmethod
    @overload
    def create_arm(cls, config: NeroCanV121Config, **kwargs) -> NeroDriverV121:
        """Nero CAN driver for firmware >= v121 (1.21)."""
        ...


