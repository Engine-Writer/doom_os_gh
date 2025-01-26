import os

def list_directory_contents(base_path, ignore_dirs=None):
    if ignore_dirs is None:
        ignore_dirs = {'.git', '.vscode'}
    dir_structure = []
    for root, dirs, files in os.walk(base_path):
        # Exclude ignored directories
        dirs[:] = [d for d in dirs if d not in ignore_dirs]
        
        relative_root = os.path.relpath(root, base_path)
        if relative_root == '.':
            relative_root = base_path
        
        # Add directories and files to the structure
        dir_structure.append({"path": relative_root, "files": files})
    return dir_structure

# Get the current working directory
cwd = os.getcwd()
directory_contents = list_directory_contents(cwd)

# Output the directory and file structure in a format understandable to the assistant
print("The directory and file structure is as follows (excluding .git/ and .vscode/):")
for entry in directory_contents:
    print(f"Directory: {entry['path']}")
    for file in entry['files']:
        print(f"  File: {file}")
