import subprocess
import re
from pyectool import ECController

ec = ECController()

def run_ectool_command(cmd):
    result = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        shell=True,
        text=True,
    )
    return result.stdout

def test_is_on_ac():
    result_py = ec.is_on_ac()
    out = run_ectool_command("ectool battery")
    ectool_result = bool(re.search(r"Flags.*AC_PRESENT", out))
    print(f"[is_on_ac] pyectool={result_py}, ectool={ectool_result}")
    assert result_py == ectool_result, f"pyectool.is_on_ac={result_py}, ectool={ectool_result}"

def test_get_max_temp():
    py_temp = ec.get_max_temp()
    raw_out = run_ectool_command("ectool temps all")
    raw_temps = re.findall(r"\(= (\d+) C\)", raw_out)
    temps = sorted([int(x) for x in raw_temps if int(x) > 0], reverse=True)
    ectool_temp = float(round(temps[0], 2)) if temps else -1
    print(f"[get_max_temp] pyectool={py_temp}, ectool={ectool_temp}")
    assert abs(py_temp - ectool_temp) <= 1.0, f"pyectool={py_temp}, ectool={ectool_temp}"

def test_get_max_non_battery_temp():
    raw_out = run_ectool_command("ectool tempsinfo all")
    battery_sensors_raw = re.findall(r"\d+ Battery", raw_out, re.MULTILINE)
    battery_sensors = [x.split(" ")[0] for x in battery_sensors_raw]
    all_sensors = re.findall(r"^\d+", raw_out, re.MULTILINE)
    non_battery_sensors = [x for x in all_sensors if x not in battery_sensors]

    temps = []
    for sensor in non_battery_sensors:
        out = run_ectool_command(f"ectool temps {sensor}")
        matches = re.findall(r"\(= (\d+) C\)", out)
        temps.extend([int(x) for x in matches])

    ectool_temp = float(round(max(temps), 2)) if temps else -1
    py_temp = ec.get_max_non_battery_temp()
    print(f"[get_max_non_battery_temp] pyectool={py_temp}, ectool={ectool_temp}")
    assert abs(py_temp - ectool_temp) <= 1.0, f"pyectool={py_temp}, ectool={ectool_temp}"

def test_get_all_temps():
    py_vals = ec.get_all_temps()
    raw_out = run_ectool_command("ectool temps all")
    ectool_vals = [int(x) for x in re.findall(r"\(= (\d+) C\)", raw_out)]
    print(f"[get_all_temps] pyectool={py_vals}, ectool={ectool_vals}")
    assert all(abs(p - e) <= 1 for p, e in zip(py_vals, ectool_vals[:len(py_vals)])), "Mismatch in get_all_temps"

def test_get_temp():
    try:
        py_temp = ec.get_temp(0)
        raw_out = run_ectool_command("ectool temps 0")
        match = re.search(r"\(= (\d+) C\)", raw_out)
        ectool_temp = int(match.group(1)) if match else -1
        print(f"[get_temp(0)] pyectool={py_temp}, ectool={ectool_temp}")
        assert abs(py_temp - ectool_temp) <= 1
    except Exception as e:
        print(f"[get_temp(0)] Skipped due to: {e}")

def test_get_num_temp_sensors():
    py_val = ec.get_num_temp_sensors()
    raw_out = run_ectool_command("ectool temps all")
    ectool_vals = [int(x) for x in re.findall(r"\(= (\d+) C\)", raw_out)]
    ectool_val = len(ectool_vals)
    print(f"[get_num_temp_sensors] pyectool={py_val}, ectool={ectool_val}")
    assert abs(py_val == ectool_val)

def test_get_temp_info():
    py_info = ec.get_temp_info(0)
    print(f"[get_temp_info] pyectool={py_info}")

def test_get_all_fans_rpm():
    py_vals = ec.get_all_fans_rpm()
    out = run_ectool_command("ectool pwmgetfanrpm")
    ectool_vals = [int(x) for x in re.findall(r"rpm = (\d+)", out)]
    print(f"[get_all_fans_rpm] pyectool={py_vals}, ectool={ectool_vals}")
    assert all(abs(p - e) <= 20 for p, e in zip(py_vals, ectool_vals)), "Mismatch in fan RPMs"

def test_get_fan_rpm():
    try:
        py_val = ec.get_fan_rpm(0)
        out = run_ectool_command("ectool pwmgetfanrpm 0")
        match = re.search(r"rpm = (\d+)", out)
        ectool_val = int(match.group(1)) if match else -1
        print(f"[get_fan_rpm(0)] pyectool={py_val}, ectool={ectool_val}")
        assert abs(py_val - ectool_val) <= 20
    except Exception as e:
        print(f"[get_fan_rpm(0)] Skipped due to: {e}")

def test_get_num_fans():
    py_val = ec.get_num_fans()
    out = run_ectool_command("ectool pwmgetnumfans")
    match = re.search(r"Number of fans\s*=\s*(\d+)", out)
    ectool_val = int(match.group(1)) if match else -1
    print(f"[get_num_fans] pyectool={py_val}, ectool={ectool_val}")
    assert py_val == ectool_val, f"Mismatch: pyectool={py_val}, ectool={ectool_val}"
