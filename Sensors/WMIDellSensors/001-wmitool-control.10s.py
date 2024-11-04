#!/usr/local/bin/python3.12
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
        parser.add_argument("--hibernate", help = "Hibernate macOS", nargs='?', const=1, type=int)
        parser.add_argument("--auto", help = "Auto fan mode", nargs='?', const=1, type=int)
        parser.add_argument("--manual", help = "Manual fan mode", nargs='?', const=1, type=int)
        parser.add_argument("--leftoff", help = "Left fan off", nargs='?', const=1, type=int)
        parser.add_argument("--leftmedium", help = "Left fan medium", nargs='?', const=1, type=int)
        parser.add_argument("--lefthigh", help = "Left fan high", nargs='?', const=1, type=int)
        parser.add_argument("--rightoff", help = "Right fan off", nargs='?', const=1, type=int)
        parser.add_argument("--rightmedium", help = "Right fan medium", nargs='?', const=1, type=int)
        parser.add_argument("--righthigh", help = "Right fan high", nargs='?', const=1, type=int)
        parser.add_argument("--primarily_ac", help = "Set charging mode to PRIMARILY_AC", nargs='?', const=1, type=int)
        parser.add_argument("--adaptive", help = "Set charging mode to ADAPTIVE", nargs='?', const=1, type=int)
        parser.add_argument("--custom", help = "Set charging mode to CUSTOM", nargs='?', const=1, type=int)
        parser.add_argument("--standard", help = "Set charging mode to CUSTOM", nargs='?', const=1, type=int)
        parser.add_argument("--express", help = "Set charging mode to EXPRESS", nargs='?', const=1, type=int)

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
            
        hibernate = getattr(args, "hibernate")
        if hibernate != None:
            cmd = ("/bin/bash", "/Users/sergey/.sleep")
            p = subprocess.run(cmd, capture_output=False, text=True)
            cmd = ("/usr/local/bin/wmitool", "--raw", "32000", "0")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        primarily_ac = getattr(args, "primarily_ac")
        if primarily_ac != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "1", "0", "0x0341", "3")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        cmd = ("/usr/local/bin/wmitool", "--raw", "0", "0", "0x0341")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        primarily_ac_active = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           res1 = results[1].replace("res[1] = ", "").strip()
           res2 = results[2].replace("res[2] = ", "").strip()
           primarily_ac_active = int(res1) == 3 and int(res2) == 3

        adaptive = getattr(args, "adaptive")
        if adaptive != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "1", "0", "0x0342", "4")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        cmd = ("/usr/local/bin/wmitool", "--raw", "0", "0", "0x0342")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        adaptive_active = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           res1 = results[1].replace("res[1] = ", "").strip()
           res2 = results[2].replace("res[2] = ", "").strip()
           adaptive_active = int(res1) == 4 and int(res2) == 4
            
        custom = getattr(args, "custom")
        if custom != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "1", "0", "0x0343", "5")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        cmd = ("/usr/local/bin/wmitool", "--raw", "0", "0", "0x0343")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        custom_active = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           res1 = results[1].replace("res[1] = ", "").strip()
           res2 = results[2].replace("res[2] = ", "").strip()
           custom_active = int(res1) == 5 and int(res2) == 5
           
        cmd = ("/usr/local/bin/wmitool", "--raw", "0", "0", "0x0349")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        custom_from = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           res1 = results[1].replace("res[1] = ", "").strip()
           res2 = results[2].replace("res[2] = ", "").strip()
           custom_from = int(res1)
           
        cmd = ("/usr/local/bin/wmitool", "--raw", "0", "0", "0x034A")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        custom_to = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           res1 = results[1].replace("res[1] = ", "").strip()
           res2 = results[2].replace("res[2] = ", "").strip()
           custom_to = int(res1)
            
        standard = getattr(args, "standard")
        if standard != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "1", "0", "0x0346", "1")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
          
        cmd = ("/usr/local/bin/wmitool", "--raw", "0", "0", "0x0346")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        standard_active = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           res1 = results[1].replace("res[1] = ", "").strip()
           res2 = results[2].replace("res[2] = ", "").strip()
           standard_active = int(res1) == 1 and int(res2) == 1
          
        express = getattr(args, "express")
        if express != None:
            cmd = ("/usr/local/bin/wmitool", "--raw", "1", "0", "0x0347", "2")
            p = subprocess.run(cmd, capture_output=True, text=True)
            p.check_returncode()
            
        cmd = ("/usr/local/bin/wmitool", "--raw", "0", "0", "0x0347")
        p = subprocess.run(cmd, capture_output=True, text=True)
        p.check_returncode()
        express_active = None
        if p.stdout is not None:
           output = p.stdout
           output.replace("actionEvaluate returns ", "")
           results = output.split(',')
           assert len(results) == 4
           res1 = results[1].replace("res[1] = ", "").strip()
           res2 = results[2].replace("res[2] = ", "").strip()
           express_active = int(res1) == 2 and int(res2) == 2
            
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
        print(f'Hibernate macOS | refresh=false bash="{script_path()}" param1=--hibernate terminal=false')
        
        print("---")
        if standard_active:
            print(f'✓ STANDARD')
        else:
            print(f'STANDARD | refresh=true bash="{script_path()}" param1=--standard terminal=false')
        if express_active:
            print(f'✓ EXPRESS')
        else:
            print(f'EXPRESS | refresh=true bash="{script_path()}" param1=--express terminal=false')
        if primarily_ac_active:
            print(f'✓ PRIMARILY AC')
        else:
            print(f'PRIMARILY AC | refresh=true bash="{script_path()}" param1=--primarily_ac terminal=false')
        if adaptive_active:
            print(f'✓ ADAPTIVE')
        else:
            print(f'ADAPTIVE | refresh=true bash="{script_path()}" param1=--adaptive terminal=false')
        if custom_active:
            print(f'✓ CUSTOM from {custom_from} to {custom_to}'.format(custom_from=custom_from, custom_to=custom_to))
        else:
            print(f'CUSTOM from {custom_from} to {custom_to} | refresh=true bash="{script_path()}" param1=--custom terminal=false'.format(custom_from=custom_from, custom_to=custom_to))
            
        print('-- 50 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=50 param6=50 terminal=false')
        print('-- 55 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=55 param6=55 terminal=false')
        print('-- 60 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=60 param6=60 terminal=false')
        print('-- 65 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=65 param6=65 terminal=false')
        print('-- 70 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=70 param6=70 terminal=false')
        print('-- 75 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=75 param6=75 terminal=false')
        print('-- 80 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=80 param6=80 terminal=false')
        print('-- 85 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=85 param6=85 terminal=false')
        print('-- 90 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=90 param6=90 terminal=false')
        print('-- 95 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x349 param5=95 param6=95 terminal=false')
        print('-- ---')
        print('-- 55 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=55 param6=55 terminal=false')
        print('-- 60 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=60 param6=60 terminal=false')
        print('-- 65 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=65 param6=65 terminal=false')
        print('-- 70 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=70 param6=70 terminal=false')
        print('-- 75 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=75 param6=75 terminal=false')
        print('-- 80 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=80 param6=80 terminal=false')
        print('-- 85 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=85 param6=85 terminal=false')
        print('-- 90 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=90 param6=90 terminal=false')
        print('-- 95 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=95 param6=95 terminal=false')
        print('-- 100 | refresh=true bash="/usr/local/bin/wmitool" param1=--raw param2=1 param3=0 param4=0x34A param5=100 param6=100 terminal=false')
            
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

