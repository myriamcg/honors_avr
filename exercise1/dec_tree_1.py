# import llvmlite.binding as llvm
# import networkx as nx
# from collections import defaultdict

# # Initialize LLVM
# llvm.initialize()
# llvm.initialize_native_target()
# llvm.initialize_native_asmprinter()

# def parse_ll_file(file_path):
#     # Parse the LLVM IR file
#     with open(file_path, 'r') as f:
#         llvm_ir = f.read()

#     # Parse the LLVM IR module
#     llvm_module = llvm.parse_assembly(llvm_ir)
#     llvm_module.verify()
#     return llvm_module
# def build_cfg(llvm_module):
#     # Create a directed graph for the CFG
#     cfg = nx.DiGraph()

#     # Iterate through functions in the module
#     for func in llvm_module.functions:
#         if func.is_declaration:  # Skip declarations
#             continue

#         # Add function as a node
#         cfg.add_node(func.name, type="function")

#         # Process each basic block in the function
#         for block in func.blocks:
#             block_name = f"{func.name}:{block.name}"
#             cfg.add_node(block_name, type="basic_block")

#             # Convert the instructions iterator to a list
#             instructions = list(block.instructions)

#             # Analyze the terminator instruction to determine successors
#             if instructions:  # Ensure the block has instructions
#                 terminator = instructions[-1]  # Last instruction in the block
#                 if terminator.opcode == "br":  # Branch instruction
#                     for operand in terminator.operands:
#                         # Check if the operand is a basic block reference
#                         if operand.name:  # Basic block names are non-empty
#                             successor_name = f"{func.name}:{operand.name}"
#                             cfg.add_edge(block_name, successor_name)
#                 elif terminator.opcode == "switch":  # Switch instruction
#                     for case in terminator.operands[1:]:  # Skip the condition operand
#                         if case.name:  # Basic block names are non-empty
#                             successor_name = f"{func.name}:{case.name}"
#                             cfg.add_edge(block_name, successor_name)

#     return cfg

# def convert_to_tree(cfg, root):
#     # Convert graph to tree (basic example)
#     tree = defaultdict(list)

#     def dfs(node, parent=None):
#         for neighbor in cfg.successors(node):
#             tree[node].append(neighbor)
#             dfs(neighbor, node)

#     dfs(root)
#     return tree



# # Main
# if __name__ == "__main__":
#     # Path to your .ll file
#     ll_file = "prog.ll"

#     # Parse LLVM IR and build CFG
#     llvm_module = parse_ll_file(ll_file)
#     cfg = build_cfg(llvm_module)
#     print("si si")
#     print(cfg.nodes())

#     # Assuming "main" function is the entry point
#     root = "main"

#     # Convert to tree structure
#     cfg_tree = convert_to_tree(cfg, root)

#     # Print the tree structure
#     print(cfg_tree)

import re
from collections import defaultdict

# Read the LLVM IR file
with open("prog.ll", "r") as file:
    content = file.read()


function_definitions = re.findall(r'define.*?@(\w+)\(.*?\).*?\{(.*?)\}', content, re.S)

function_analysis = defaultdict(lambda: {"branches": [], "switches": [], "calls": []})

for func_name, func_body in function_definitions:
    branches = re.findall(r'br\s+i1\s+(\%\w+)', func_body)
    
    # Find all switch statements
    switches = re.findall(r'switch\s+\w+\s+(\%\w+),', func_body)
    
    # Find all function calls
    # calls = re.findall(r'call\s+\w+.*?@(\w+)\(', func_body)
    calls = re.findall(r'(?:call|invoke)\s+[^{@]*@(\w+)\(', func_body)
    
    # Store the data
    function_analysis[func_name]["branches"] = branches
    function_analysis[func_name]["switches"] = switches
    function_analysis[func_name]["calls"] = calls

# Print the results
for func, analysis in function_analysis.items():
    print(f"Function '{func}':")
    
    branches = analysis["branches"]
    if branches:
        print(f"  Conditional branches: {', '.join(branches)}")
    else:
        print("  No conditional branches found.")
    
    switches = analysis["switches"]
    if switches:
        print(f"  Switch conditions: {', '.join(switches)}")
    else:
        print("  No switch conditions found.")
    
    calls = analysis["calls"]
    if calls:
        print(f"  Function calls: {', '.join(calls)}")
    else:
        print("  No function calls found.")

