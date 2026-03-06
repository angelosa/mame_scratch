"""Generate a PCI card template file for MAME use

https://wiki.osdev.org/PCI

All hexadecimals inputs are intended without the 0x part
"""

import sys
import os
from jinja2 import Environment, FileSystemLoader

if len(sys.argv) != 2:
    print("Usage: pci_card.py <destination_folder>", file=sys.stderr)
    sys.exit(2)

DEST_PATH = sys.argv[1]

if not os.path.isdir(DEST_PATH):
    print(f"Error: {DEST_PATH} is not a directory", file=sys.stderr)
    sys.exit(1)

environment = Environment(loader=FileSystemLoader("templates"))
template_cpp = environment.get_template("pci_card.cpp.jinja")
template_h = environment.get_template("pci_card.h.jinja")

context_input = {
    "KLASS": input("klass (_device prefix):"),
    "DEVICE": input("device (uppercase):").upper(),
    "LONGNAME": input("device long name:"),
    "VENDORID": input("PCI vendor ID:"),
    "DEVICEID": input("PCI device ID:"),
    "COMMAND_MASK": input("""Command mask
    (settable flags in PCI register $04, enter for default to 0):""") or "0",
    "STATUS": input("""Status startup default
    (PCI register $06, enter for default to 0):""") or "0",
    "KLASSCODE": input("PCI Class Code (PCI register $09):"),
    "CAPPTR": input("""Has Capability List?
    (PCI register $34, default capptr_r pointer number, enter to omit):""") or "0",
    "INTR_PIN": input("""Has interrupt pin? (PCI register $3d)
    (1=INTA#, 2=INTB#, 3=INTC#, 4=INTD#, enter for default to 0=no pin):""") or "0",
    "MIN_GNT": input("Minimum Grant? (PCI register $3e, enter to omit):") or "0",
    "MAX_LAT": input("Maximum Latency? (PCI register $3f, enter to omit):") or "0",
    "MAIN": input("""Device main type for initial unsupported flag
    (GRAPHICS/SOUND/..., cfr. src/emu/device.h):""").upper(),
    "LICENSE": input("SPDX license (enter for default BSD-3-Clause):") or "BSD-3-Clause"
}

intr_pin_name = {
    "0": "",
    "1": "INTA#",
    "2": "INTB#",
    "3": "INTC#",
    "4": "INTD#"
}

context = context_input | { "INTR_PIN_NAME": intr_pin_name[context_input["INTR_PIN"]] }

result_cpp = template_cpp.render(context)
result_h = template_h.render(context)

with open(os.path.join(DEST_PATH, context["KLASS"] + ".cpp"),
        mode="w", encoding="utf-8") as results:
    results.write(result_cpp)

with open(os.path.join(DEST_PATH, context["KLASS"] + ".h"),
        mode="w", encoding="utf-8") as results:
    results.write(result_h)

print("TBD: implement pci_slot override")
print("TBD: implement scripts/src/bus.lua override")
