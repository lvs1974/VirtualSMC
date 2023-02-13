#!/usr/bin/python3
# coding=utf-8

# <xbar.title>Dell WMI Control</xbar.title>
# <xbar.version>v1.0</xbar.version>
# <xbar.author>Sergey Lvov</xbar.author>
# <xbar.author.github></xbar.author.github>
# <xbar.desc>Dell WMI Control widget</xbar.desc>
# <xbar.image></xbar.image>
# <xbar.dependencies>python</xbar.dependencies>

import subprocess
import sys
import os
import argparse

def script_path():
    return os.path.realpath(__file__)

def script_name():
    return os.path.basename(__file__)


class App:
    class Mode:
        def __init__(self, name, runner, help, is_default):
            self.name = name
            self.runner = runner
            self.help = help
            self.is_default = is_default

    def __init__(self):
        pass

    def run(self):
        # Initialize parser
        parser = argparse.ArgumentParser()
                
        # Adding optional argument
        parser.add_argument("-s", "--set-thermal-mode", help = "Set Thermal Mode")

        # Read arguments from command line
        args = parser.parse_args()
                        
        new_mode = getattr(args, "set_thermal_mode")
        if new_mode != None:
            cmd = ("/usr/local/bin/wmitool", "--set-thermal-mode={}".format(str(new_mode)))
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return 0 if "successfully" in p.stdout else 1
        
        cmd = ("/usr/local/bin/wmitool", "-g")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        if p.stdout is not None:
           if "Balanced" in  p.stdout:
               print("Balanced")
           if "Cool Bottom" in  p.stdout:
               print("Cool Bottom")
           if "Quiet" in  p.stdout:
               print("Quiet")
           if "Performance" in p.stdout:
               print("Performance")

        print("---")
        print(f'Balanced    | refresh=true bash="{script_path()}" param1=--set-thermal-mode=balanced terminal=false')
        print(f'Cool Bottom | refresh=true bash="{script_path()}" param1=--set-thermal-mode=cool-bottom terminal=false')
        print(f'Quiet       | refresh=true bash="{script_path()}" param1=--set-thermal-mode=quiet terminal=false')
        print(f'Performance | refresh=true bash="{script_path()}" param1=--set-thermal-mode=performance terminal=false')
        return 0

app = App()

if __name__ == '__main__':
    sys.exit(app.run())

