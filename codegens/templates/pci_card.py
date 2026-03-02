"""Generate a PCI card template file
"""

from jinja2 import Environment, FileSystemLoader

environment = Environment(loader=FileSystemLoader("templates"))
template_cpp = environment.get_template("pci_card.cpp.jinja")
context = {
    "KLASS": input("klass (_device prefix):"),
    "DEVICE": input("device (uppercase):").upper(),
    "LONGNAME": input("device long name:"),
    "VENDORID": input("PCI vendor ID:"),
    "DEVICEID": input("PCI device ID:"),
    "KLASSCODE": input("PCI Class Code (PCI register 09):"),
    "MAIN": input("Device main type (GRAPHICS/SOUND/MACHINE):").upper(),
}

result_cpp = template_cpp.render(context)
template_h = environment.get_template("pci_card.h.jinja")
result_h = template_h.render(context)

with open(context["KLASS"] + ".cpp", mode="w", encoding="utf-8") as results:
    results.write(result_cpp)

with open(context["KLASS"] + ".h", mode="w", encoding="utf-8") as results:
    results.write(result_h)

print("TBD: automatically move above to the actual folder")
print("TBD: implement pci_slot override")
print("TBD: implement scripts/src/bus.lua override")
