import json
import os
import subprocess

current_directory = os.getcwd()

c_cpp_files = []

for root, dirs, files in os.walk(current_directory):
    for file in files:
        if file.lower().endswith(('.c', '.cpp', '.h', '.hpp', '.cc', '.cxx', '.hh', '.hxx')):
            file_path = os.path.join(root, file)
            c_cpp_files.append(file_path)

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

    functions = []
    for c in ctags_output.splitlines():
        info = json.loads(c)
        functions.append({
            "name": info["name"],
            "start_line": int(info["line"]),
            "end_line": int(info["end"])
        })

    printed_functions = set()
    stack = []

    function_code_map = {}
    for function in functions:
        func_name = function["name"]
        start_line = function["start_line"]
        end_line = function["end_line"]

        function_code = subprocess.run([
                "sed", "-n", f"{start_line},{end_line}p", file
            ],
            check=True,
            capture_output=True
        ).stdout.decode("utf-8")

        # Store the function code in the map with a (file, function_name) key
        function_code_map[(file, func_name)] = function_code
        print("function code is ")
        print(function_code)
       
    for function in functions:
        func_name = function["name"]
        start_line = function["start_line"]
        end_line = function["end_line"]

        for l in lines.splitlines():
            split = l.split(':')
            problematic_file = split[0]
            issue_line = int(split[1])

            if problematic_file == file and start_line <= issue_line <= end_line or func_name == 'main':
                if func_name not in printed_functions:
                    print(f"fun: {func_name}")
                    printed_functions.add(func_name)
                    stack.append(function)
                break

    while stack:
        current_function = stack.pop()
        current_func_name = current_function["name"]

        for function in functions:
            func_name = function["name"]
            if func_name in printed_functions:
                continue 

            function_code = function_code_map[(file, func_name)]
            if current_func_name in function_code:
                print(f"fun: {func_name}")
                printed_functions.add(func_name)
                stack.append(function)
