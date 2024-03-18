#!/usr/local/bin/python3
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
        parser.add_argument("--refresh", help = "Refresh all sensors", nargs='?', const=1, type=int)
        parser.add_argument("--auto", help = "Auto fan mode", nargs='?', const=1, type=int)
        parser.add_argument("--manual", help = "Manual fan mode", nargs='?', const=1, type=int)
        parser.add_argument("--leftoff", help = "Left fan off", nargs='?', const=1, type=int)
        parser.add_argument("--leftmedium", help = "Left fan medium", nargs='?', const=1, type=int)
        parser.add_argument("--lefthigh", help = "Left fan high", nargs='?', const=1, type=int)
        parser.add_argument("--rightoff", help = "Right fan off", nargs='?', const=1, type=int)
        parser.add_argument("--rightmedium", help = "Right fan medium", nargs='?', const=1, type=int)
        parser.add_argument("--righthigh", help = "Right fan high", nargs='?', const=1, type=int)

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
               
        raw_mode = getattr(args, "refresh")
        if raw_mode != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "33000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        auto_mode = getattr(args, "auto")
        if auto_mode != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "34000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        manual_mode = getattr(args, "manual")
        if manual_mode != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "35000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        left_off = getattr(args, "leftoff")
        if left_off != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "36000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        left_medium = getattr(args, "leftmedium")
        if left_medium != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "37000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        left_high = getattr(args, "lefthigh")
        if left_high != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "38000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return

        right_off = getattr(args, "rightoff")
        if right_off != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "39000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        right_medium = getattr(args, "rightmedium")
        if right_medium != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "40000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        right_high = getattr(args, "righthigh")
        if right_high != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "41000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            return
            
        print("---")
        print(f'Refresh sensors | refresh=false bash="{script_path()}" param1=--refresh terminal=false')

        print("---")
        print(f'Auto fan mode | refresh=false bash="{script_path()}" param1=--auto terminal=false')
        print(f'Manual fan mode | refresh=false bash="{script_path()}" param1=--manual terminal=false')

        print("---")
        print(f'Left fan off | refresh=false bash="{script_path()}" param1=--leftoff terminal=false')
        print(f'Left fan medium | refresh=false bash="{script_path()}" param1=--leftmedium terminal=false')
        print(f'Left fan high | refresh=false bash="{script_path()}" param1=--lefthigh terminal=false')
        
        print("---")
        print(f'Right fan off | refresh=false bash="{script_path()}" param1=--rightoff terminal=false')
        print(f'Right fan medium | refresh=false bash="{script_path()}" param1=--rightmedium terminal=false')
        print(f'Right fan high | refresh=false bash="{script_path()}" param1=--righthigh terminal=false')

app = App()

if __name__ == '__main__':
    sys.exit(app.run())

