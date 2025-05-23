import json
import os
import subprocess

# Get the current directory
current_directory = os.getcwd()

# List to hold all C/C++ files
c_cpp_files = []

# Walk through the directory tree
for root, dirs, files in os.walk(current_directory):
    for file in files:
        if file.lower().endswith(('.c', '.cpp', '.h', '.hpp', '.cc', '.cxx', '.hh', '.hxx')):
            # Construct the full file path
            file_path = os.path.join(root, file)
            c_cpp_files.append(file_path)

# Print all C and C++ files
for file in c_cpp_files:
    completed = subprocess.run([
            "cppcheck",
            "--enable=all",
            "--inline-suppr",
            "--suppress=missingInclude",
            "--force",
            '--template={file}:{line}',
            file
        ],
        check=True,
        capture_output=True
    )

    lines = completed.stderr.decode("utf-8")

    if lines == "":
        continue

    ctags_output = subprocess.run([
            "ctags",
            "--c-kinds=f",
            "--fields={name}{line}{end}",
            "--output-format=json",
            "--sort=no",
            file,
        ],
        check=True,
        capture_output=True
    ).stdout.decode("utf-8")

    main_printed = False


    for l in lines.splitlines():
        split = l.split(':')
        problematic_file = split[0]
        line = int(split[1])

        if problematic_file != file:
            continue

        for c in ctags_output.splitlines():
            info = json.loads(c)
            fname = info["name"]
            if fname == "main" and not main_printed:
             print(f"fun: {fname}")
             main_printed = True
            line_number = int(info["line"])
            end_line = int(info["end"])
            if line_number <= line <= end_line:
                print(f"fun: {fname}, line_number: {line_number}, end_line: {end_line}")
                break
