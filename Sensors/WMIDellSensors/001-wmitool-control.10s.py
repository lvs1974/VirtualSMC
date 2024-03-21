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
            
        auto_mode = getattr(args, "auto")
        if auto_mode != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "34000", "0", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        manual_mode = getattr(args, "manual")
        if manual_mode != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "34000", "0", "0xffff")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        left_off = getattr(args, "leftoff")
        if left_off != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "37000", "0", "0", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        left_medium = getattr(args, "leftmedium")
        if left_medium != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "37000", "0", "0", "1")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        left_high = getattr(args, "lefthigh")
        if left_high != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "37000", "0", "0", "2")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()

        right_off = getattr(args, "rightoff")
        if right_off != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "37000", "0", "1", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        right_medium = getattr(args, "rightmedium")
        if right_medium != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "37000", "0", "1", "1")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        right_high = getattr(args, "righthigh")
        if right_high != None:
            cmd = ("/usr/local/bin/wmitool", "--raw","37000", "0", "1", "2")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        cmd = ("/usr/local/bin/wmitool", "--raw", "36000", "0", "0")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        fan_mode = None
        fan_left_status = None
        fan_right_status = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           fan_mode = results[1].replace("res[1] = ", "").strip()	
           fan_left_status = results[2].replace("res[2] = ", "").strip()
           fan_right_status = results[3].replace("res[3] = ", "").strip()

            
        print("---")
        print(f'Refresh sensors | refresh=false bash="{script_path()}" param1=--refresh terminal=false')

        print("---")

        if fan_mode == "0":
            print(f'✓ Auto fan mode')
        else:
            print(f'Auto fan mode | refresh=true bash="{script_path()}" param1=--auto terminal=false')
            
        if fan_mode != "0":
            print(f'✓ Manual fan mode')
        else:
            print(f'Manual fan mode | refresh=true bash="{script_path()}" param1=--manual terminal=false')

        print("---")
        if fan_left_status == "0":
            print(f'✓ Left fan off')
        else:
            print(f'Left fan off | refresh=true bash="{script_path()}" param1=--leftoff terminal=false')
        if fan_left_status == "1":
            print(f'✓ Left fan medium')
        else:
            print(f'Left fan medium | refresh=true bash="{script_path()}" param1=--leftmedium terminal=false')
        if fan_left_status == "2":
            print(f'✓ Left fan high')
        else:
            print(f'Left fan high | refresh=true bash="{script_path()}" param1=--lefthigh terminal=false')
        
        print("---")
        if fan_right_status == "0":
            print(f'✓ Right fan off')
        else:
            print(f'Right fan off | refresh=true bash="{script_path()}" param1=--rightoff terminal=false')
        if fan_right_status == "1":
            print(f'✓ Right fan medium')
        else:
            print(f'Right fan medium | refresh=true bash="{script_path()}" param1=--rightmedium terminal=false')
        if fan_right_status == "2":
            print(f'✓ Right fan high')
        else:
            print(f'Right fan high | refresh=true bash="{script_path()}" param1=--righthigh terminal=false')

app = App()

if __name__ == '__main__':
    sys.exit(app.run())

