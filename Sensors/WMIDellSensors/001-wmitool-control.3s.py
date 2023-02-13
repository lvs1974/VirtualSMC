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

        cmd = ("/usr/local/bin/wmitool", "-g")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        mode = None
        modes = ["Balanced", "Cool Bottom", "Quiet", "Performance"]
        if p.stdout is not None:
           for current_mode in modes:
               if current_mode in  p.stdout:
                  mode = current_mode

        print(mode if mode is not None else "Unknown")
        print("---")
        
        for current_mode in modes:
            if mode == current_mode:
               print(f'✓ {mode}'.format(mode=mode))
            else:
               param = current_mode.replace(' ', '-').lower()
               print(f'{current_mode}    | refresh=true bash="{script_path()}" param1=--set-thermal-mode={param} terminal=false'.format(mode=current_mode, param=param))

app = App()

if __name__ == '__main__':
    sys.exit(app.run())

