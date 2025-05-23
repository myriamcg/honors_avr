import json
import os
import subprocess

def parse_code(function_name, function_code):
    structure = {
        "function_name": function_name,
        "calls": [],
        "conditionals": [],
        "loops": []
    }

    lines = function_code.splitlines()

    for line in lines:
        stripped = line.strip()
        # Skip empty lines and comments
        if not stripped or stripped.startswith("//") or stripped.startswith("/*"):
            continue
        
        # Detect function calls (basic detection, improve if necessary)
        if "(" in stripped and stripped.endswith(";"):
            parts = stripped.split("(")
            if parts and parts[0].strip():  # Ensure there's content before the parenthesis
                words = parts[0].split()
                if words:  # Ensure the split result is not empty
                    call = words[-1]  # The last word is usually the function name
                    structure["calls"].append(call)
        
        if stripped.startswith("if") or stripped.startswith("else if") or stripped.startswith("else"):
            structure["conditionals"].append(stripped)
        
        
        if stripped.startswith("for") or stripped.startswith("while"):
            structure["loops"].append(stripped)

    return structure

def build_decision_tree(functions, relationships):
    def build_tree(node, visited):
        if node in visited:
            return {"name": node, "calls": []}  # Prevent infinite recursion
        visited.add(node)
        return {
            "name": node,
            "calls": [build_tree(child, visited) for child in relationships[node]]
        }

    tree = []
    visited = set()
    for function in functions:
        if function not in visited:
            tree.append(build_tree(function, visited))
    return tree


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
        if file.endswith(".cpp") and "CMake" not in file: 
            functions.append({
                "name": info["name"],
                "start_line": int(info["line"]),
                "end_line": int(info["end"])
            })

    
    function_code_map = {}
    function_structures = {}
    printed_functions = set()
    stack = []
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
        function_structures[info["name"]] = parse_code(info["name"], function_code)
       
    for function in functions:
        func_name = function["name"]
        start_line = function["start_line"]
        end_line = function["end_line"]
