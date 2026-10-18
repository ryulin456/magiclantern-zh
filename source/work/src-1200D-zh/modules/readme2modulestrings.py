# extract module strings from README.rst

import sys, re
import subprocess
from datetime import datetime

def _c(value):
    return '"' + value.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n') + '"'

_lines = open("README.rst", encoding="utf-8").read().replace("\r\n", "\n").splitlines()
_pairs = [("Name", _lines[0].strip())]
_body = []
for _line in _lines[2:]:
    _m = re.match(r"^:([^:]+):\s*(.*)$", _line.strip())
    if _m:
        if not (_m.group(2).startswith("<") and _m.group(2).endswith(">")):
            _pairs.append((_m.group(1).strip(), _m.group(2).strip()))
        continue
    if _line.lstrip().startswith(".."):
        continue
    if re.match(r"^[=\-~`^:#*+]+$", _line.strip()):
        continue
    _body.append(_line.rstrip())
_description = "\n".join(_body).strip()
while "\n\n\n" in _description:
    _description = _description.replace("\n\n\n", "\n\n")
_pairs.append(("Description", _description))
for _i, (_name, _value) in enumerate(_pairs):
    _id = chr(ord('a') + _i)
    print("static char __module_string_%s_name [] MODULE_STRINGS_SECTION = %s;" % (_id, _c(_name)))
    print("static char __module_string_%s_value[] MODULE_STRINGS_SECTION = %s;" % (_id, _c(_value)))
print("\n#define MODULE_STRINGS() \\")
print("  MODULE_STRINGS_START() \\")
for _i in range(len(_pairs)):
    _id = chr(ord('a') + _i)
    print("  MODULE_STRING(__module_string_%s_name, __module_string_%s_value) \\" % (_id, _id))
print("  MODULE_STRINGS_END()")
sys.exit(0)

from align_string_proportional import word_wrap
from rbf_read import extent_func, rbf_init_font
rbf_init_font("../../data/fonts/argnor23.rbf")

def run(cmd):
    try:
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out = p.communicate()
        if p.returncode or out[1]:
            print(cmd, file=sys.stderr)
            print(out[0], file=sys.stderr)
            print(out[1], file=sys.stderr)
            exit(1)
        return out[0]
    except:
        print(sys.exc_info(), file=sys.stderr)
        exit(1)

# see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=37506 for how to place some strings in a custom section
# (you must declare all strings as variables, not only the pointers)

def c_repr(name):
    if "\n" in name:
        s = "\n"
        for l in name.split("\n"):
            s += "    %s\n" % ('"%s\\n"' % l.replace('"', r'\"'))
        return s
    else:
        return '"%s"' % name.replace('"', r'\"')

strings = []
def add_string(name, value):
    a = chr(ord('a') + len(strings))
    print("static char __module_string_%s_name [] MODULE_STRINGS_SECTION = %s;" % (a, c_repr(name)))
    print("static char __module_string_%s_value[] MODULE_STRINGS_SECTION = %s;" % (a, c_repr(value)))

    strings.append((name, value))

def declare_string_section():
    print()
    print("#define MODULE_STRINGS() \\")
    print('  MODULE_STRINGS_START() \\')
    for i, s in enumerate(strings):
        a = chr(ord('a') + i)
        print("    MODULE_STRING(__module_string_%s_name, __module_string_%s_value) \\" % (a, a))
    print('  MODULE_STRINGS_END()')
    
def is_command_available(name):
    """Check if command `name` is on PATH."""
    from distutils.spawn import find_executable
    return find_executable(name) is not None

# return the first command of the list that can be found on the OS
def get_command_of(commands):
    """Return the first command of the list `commands` that can be found on the OS."""
    for command in commands: 
        if is_command_available(command) == True:
            return command
    print("\nCould not find " + "/".join(commands) + ".", file=sys.stderr)
    if "rst2html" in commands:
        print("Please install python-docutils (pip install docutils).", file=sys.stderr)
    print("", file=sys.stderr)
    exit(1)

inp = open("README.rst").read().replace("\r\n", "\n")
lines = inp.strip("\n").split("\n")
title = lines[0]

used_lines = []
for l in lines[2:]:
    if l.startswith("..") or l.strip().startswith(":"):
        continue
    
    used_lines.append(l)

inp = "\n".join(used_lines)
inp = inp.split("\n\n")

add_string("Name", title)

# extract user metadata from RST meta tags
tags = {}
for l in lines[2:]:
    l = l.strip()
    m = re.match("^:([^:]+):(.+)$", l)
    if m:
        name = m.groups()[0].strip()
        value = m.groups()[1].strip()
        if value.startswith("<") and value.endswith(">"):
            continue
        add_string(name, value)
        tags[name] = value

if "Author" not in tags and "Authors" not in tags:
    print("Warning: 'Author/Authors' tag is missing. You should tell the world who wrote your module ;)", file=sys.stderr)

if "License" not in tags:
    print("Warning: 'License' tag is missing. Under what conditions we can use your module? Can we publish modified versions?", file=sys.stderr)

if "Summary" not in tags:
    print("Warning: 'Summary' tag is missing. It should be displayed as help in the Modules tab.", file=sys.stderr)

# extract readme body:
# intro -> "Description" tag;
# each section will become "Help page 1", "Help page 2" and so on

# possible available commands to check 
rst2htmlCommands = ["rst2html", "rst2html5", "rst2html.py", "rst2html5.py"]
rst2htmlCommand = get_command_of(rst2htmlCommands)

# render the RST as html -> txt without the metadata tags
# sed command at end is because Windows inserts CR characters all over the place. Removing them should be benign on other platforms. 
txt = run('cat README.rst | grep -v -E "^:([^:])+:.+$" | ' + rst2htmlCommand + ' | python ../html2text.py -b 700 | sed "s/\r$//"')

desc = ""
last_str = "Description"
help_page_num = 0
lines_per_page = 0
for p in txt.strip("\n").split("\n")[2:]:
    if p.startswith("# "): # new section
        help_page_num += 1
        add_string(last_str, desc)
        desc = ""
        last_str = "Help page %d" % help_page_num
        print("Help page %d: %s" % (help_page_num, p.strip('# ')), file=sys.stderr)
        lines_per_page = 0
        p = p[2:].strip()
    desc += "%s\n" % p
    lines_per_page += 1
    if lines_per_page > 18:
        print("Too many lines per page\n", file=sys.stderr)
        exit(1)

add_string(last_str, desc)

# extract version info
# (prints the latest changeset that affected this module)
last_change_info = run("sh ../last_change_info.sh")

if len(last_change_info):
    last_change_date, last_changeset, author, commit_msg = last_change_info.split("\n")
    split = last_change_date.split(" ")
    seconds = float(split[0])
    last_change_date = datetime.utcfromtimestamp(seconds).strftime("%Y-%m-%d %H:%M:%S UTC")
    
    # trim changeset to 7 chars, like Bitbucket does
    last_changeset = last_changeset[:7]
    
    # trim commit msg to 700px
    size = extent_func(commit_msg)[0]
    if size > 700:
        new_size = 0
        new_msg = ""
        for c in commit_msg:
            new_size += extent_func(c)[0]
            if new_size > 700:
                break
            new_msg += c
        commit_msg = new_msg + "..."
        
    add_string("Last update", "%s on %s by %s:\n%s" % (last_changeset, last_change_date, author, commit_msg))

build_date = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC")

# echo called from python in Windows behaves differently, better to avoid it.
if sys.platform == 'win32':
	build_user = run('whoami').replace("\n", "") + "@" + run('hostname').replace("\n", "")
else:
	build_user = run("echo `whoami`@`hostname`")

add_string("Build date", build_date)
add_string("Build user", build_user)

declare_string_section()
