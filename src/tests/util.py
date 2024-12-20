import subprocess
import re

Y8_PATH = "./y8-headless"
PICO8_PATH = "./pico8"

Y8_BOOT_FOOTER = "KB at boot\n"


def execute_test_rom_y8(rom_path):
    try:
        # Get the output of the test rom for `./y8-headless rom_path`
        # This is the output that would be printed to the terminal
        exit_code = 0
        output = subprocess.check_output(
            [Y8_PATH, str(rom_path)], stderr=subprocess.STDOUT
        ).decode("utf-8")

        # remove the boot messages
        output = output[output.find(Y8_BOOT_FOOTER) + len(Y8_BOOT_FOOTER) :]

        # remove ====DONE==== ending
        output = output[: output.find("====DONE====")]

    except subprocess.CalledProcessError as grepexc:
        exit_code = grepexc.returncode
        output = grepexc.output.decode("utf-8")

    return exit_code, output


def execute_test_rom_pico8(rom_path):
    # stream over output until `====DONE====`
    # then exit the process
    proc = subprocess.Popen(
        [
            PICO8_PATH,
            "-windowed",
            "1",
            "-sound",
            "0",
            "-music",
            "0",
            "-x",
            str(rom_path),
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    )

    output = ""
    for line in proc.stdout:
        if line.startswith("====DONE===="):
            proc.kill()
            break

        output += line

    # first line in headless mode is `RUNNING: <rom_path>`
    # remove it
    output = re.sub(r"^RUNNING: .*\n", "", output)

    return output


def compare_p8_y8_outputs(p8_output, y8_output):
    # compute diff, just import the libs here lol
    import difflib

    p8_lines = p8_output.splitlines(keepends=True)
    y8_lines = y8_output.splitlines(keepends=True)

    if len(p8_lines) == len(y8_lines):
        diff = []
        for i, (p8_line, y8_line) in enumerate(zip(p8_lines, y8_lines)):
            if p8_line != y8_line:
                diff.append(f"line {i:>5}: -{p8_line}")
                diff.append(f"            +{y8_line}")
    else:
        # elif len(p8_lines) < 100 and len(y8_lines) < 100:
        diff = list(difflib.ndiff(p8_lines, y8_lines))

    assert p8_output == y8_output, f"y8 erroneously caused this diff:\n{''.join(diff)}"


def check_sanitizer_issues(output):
    assert (
        "runtime error" not in output
    ), "Sanitizer errors encountered during execution"
