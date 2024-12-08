# import re
# from collections import defaultdict

# # Read the LLVM IR file
# with open("prog.ll", "r") as file:
#     content = file.read()


# function_definitions = re.findall(r'define.*?@(\w+)\(.*?\).*?\{(.*?)\}', content, re.S)

# function_analysis = defaultdict(lambda: {"branches": [], "switches": [], "calls": []})

# for func_name, func_body in function_definitions:
#     branches = re.findall(r'br\s+i1\s+(\%\w+)', func_body)
    
#     # Find all switch statements
#     switches = re.findall(r'switch\s+\w+\s+(\%\w+),', func_body)
    
#     # Find all function calls
#     # calls = re.findall(r'call\s+\w+.*?@(\w+)\(', func_body)
#     calls = re.findall(r'(?:call|invoke)\s+[^{@]*@(\w+)\(', func_body)
    
#     # Store the data
#     function_analysis[func_name]["branches"] = branches
#     function_analysis[func_name]["switches"] = switches
#     function_analysis[func_name]["calls"] = calls

# # Print the results
# for func, analysis in function_analysis.items():
#     print(f"Function '{func}':")
    
#     branches = analysis["branches"]
#     if branches:
#         print(f"  Conditional branches: {', '.join(branches)}")
#     else:
#         print("  No conditional branches found.")
    
#     switches = analysis["switches"]
#     if switches:
#         print(f"  Switch conditions: {', '.join(switches)}")
#     else:
#         print("  No switch conditions found.")
    
#     calls = analysis["calls"]
#     if calls:
#         print(f"  Function calls: {', '.join(calls)}")
#     else:
#         print("  No function calls found.")


import os
from llvmlite import binding as llvm
import re
from collections import defaultdict

llvm.initialize()
llvm.initialize_native_target()
llvm.initialize_native_asmprinter()

llvm_ir_file = "prog.ll" 
with open(llvm_ir_file, "r") as f:
    llvm_ir = f.read()

module = llvm.parse_assembly(llvm_ir)
module.verify()

function_name = "main"
fn = module.get_function(function_name)

dot = llvm.get_function_cfg(fn)

dot_file = "function_cfg.dot"
with open(dot_file, "w") as f:
    f.write(dot)

png_file = "function_cfg.png"
os.system(f"dot -Tpng {dot_file} -o {png_file}")

print(f"Control Flow Graph (CFG) has been saved as {png_file}.")



def parse_dot_file_with_llvm(dot_file_path):
  
    node_connections = defaultdict(list)
    node_labels = {}

    with open(dot_file_path, 'r') as file:
        content = file.read()

    edges = re.findall(r'(Node0x\w+):?\w* -> (Node0x\w+);', content)
    nodes = re.findall(r'(Node0x\w+) \[.*?label="{(.*?)}"\];', content, re.DOTALL)

    for src, dest in edges:
        node_connections[src].append(dest)
    for node, label in nodes:
        # Clean up the label by removing excessive whitespace
        cleaned_label = re.sub(r'\\l', '\n', label).strip()
        cleaned_label = label
        node_labels[node] = cleaned_label

    llvm_graph = {}
    for node, children in node_connections.items():
        llvm_code = node_labels.get(node, "")
        children_codes = [node_labels.get(child, "") for child in children]
        llvm_graph[llvm_code] = children_codes
    return llvm_graph, node_labels

dot_file_path = "function_cfg.dot"
llvm_graph, node_labels = parse_dot_file_with_llvm(dot_file_path)
print("node labels are")
print(node_labels)
for llvm_code, children in llvm_graph.items():
    print("LLVM Code Block:")
    print(llvm_code)
    print("Children LLVM Blocks:")
    for child_code in children:
        print(child_code)

print("number of nodes is ")
print(len(node_labels))
    
