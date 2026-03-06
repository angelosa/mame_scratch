"""Generate an ISA non-PCI x86 chipset
"""

from jinja2 import Environment, FileSystemLoader

environment = Environment(loader=FileSystemLoader("templates"))
template_cpp = environment.get_template("x86_chipset.cpp.jinja")
context = {
    "KLASS": input("klass (_device prefix, lowercase):"),
    "DEVICE": input("device (uppercase):").upper(),
    "LONGNAME": input("device long name:"),
	"LICENSE": input("SPDX license (enter for default BSD-3-Clause)") or "BSD-3-Clause"
}

result_cpp = template_cpp.render(context)
template_h = environment.get_template("x86_chipset.h.jinja")
result_h = template_h.render(context)

with open(context["KLASS"] + ".cpp", mode="w", encoding="utf-8") as results:
    results.write(result_cpp)

with open(context["KLASS"] + ".h", mode="w", encoding="utf-8") as results:
    results.write(result_h)

print("TBD: automatically move above to the actual folder")
print("TBD: implement scripts/src/machine.lua override")
