import os
import subprocess
import sys

# Directories
SRC_DIR = os.path.abspath('./')
OBJ_DIR = os.path.abspath('./bin')
ISO_SCRIPT = os.path.abspath('./scripts/makeiso.sh')
RUN_SCRIPT = os.path.abspath('./scripts/run.sh')

# Tools
CC = 'i386-elf-gcc'
LD = 'i386-elf-ld'
ASM = 'nasm'

# Compiler flags
CFLAGS = ['-ffreestanding', '-m32', '-O2', '-g', '-I', '/home/freedomuser/shared_folder/include']  # Now a list of separate flags
ASFLAGS = '-O2 -f elf32'
LDFLAGS = '-T/home/freedomuser/shared_folder/linker.ld -O2'

# Source files
ASM_SOURCES = []
C_SOURCES = []

# Object files for asm and c separately
ASM_OBJECTS = [os.path.join(OBJ_DIR, f.replace('.asm', '.asm.o')) for f in ASM_SOURCES]
C_OBJECTS = [os.path.join(OBJ_DIR, f.replace('.c', '.c.o')) for f in C_SOURCES]

# Target binary
TARGET = os.path.join(OBJ_DIR, 'doom_os.elf')

# Path for i386-elf-gcc tools
I386_ELF_GCC_PATH = '/usr/local/i386elfgcc/bin'

def move_element(lst:list[str], value:str, index:int):
    """Moves element x to position y in the list, if it exists."""
    try:
        # Find the index of element x
        index_x = lst.index(value)
        
        # Remove the element from its current position
        lst.pop(index_x)
        
        # Insert the element at the desired position y
        lst.insert(index, value)
    except ValueError:
        print(f"Element {value} not found in the list.")
    except IndexError:
        print(f"Invalid position {index}. List size: {len(lst)}")


def get_sources() -> tuple[list[str], list[str]]:
    """Recursively get all source files (.asm and .c) while ignoring directories starting with '.'."""
    asm_files = []
    c_files = []
    for root, dirs, files in os.walk(SRC_DIR):
        # Modify dirs in-place to skip any directory that starts with a dot
        dirs[:] = [d for d in dirs if not d.startswith('.')]
        for file in files:
            if file.endswith('.asm'):
                asm_files.append(os.path.abspath(os.path.join(root, file)))
            elif file.endswith('.c'):
                c_files.append(os.path.abspath(os.path.join(root, file)))
    return asm_files, c_files


def run_command(command):
    """Helper function to run a shell command with modified environment"""
    env = os.environ.copy()
    env['PATH'] = f"{env['PATH']}:{I386_ELF_GCC_PATH}"
    tcmd = ''
    for cmd in command:
        try:
            tmpx = cmd[1][1]
            tcmd = f"{tcmd} {' '.join(cmd)}"
        except:
            tcmd = f"{tcmd} {cmd}"
    print(f"Running: {tcmd}")
    subprocess.run(tcmd, shell=True, env=env)

def build_asm():
    """Compile the ASM files"""
    for asm_file, obj_file in zip(ASM_SOURCES, ASM_OBJECTS):
        run_command([ASM, ASFLAGS, os.path.join(SRC_DIR, asm_file), '-o', obj_file])

def build_c():
    """Compile the C files"""
    for c_file, obj_file in zip(C_SOURCES, C_OBJECTS):
        run_command([CC] + CFLAGS + ['-c', os.path.join(SRC_DIR, c_file), '-o', obj_file])

def link():
    """Link the object files into a final binary"""
    run_command([LD, LDFLAGS, '-o', TARGET] + ASM_OBJECTS + C_OBJECTS)

def create_iso():
    """Create the ISO image"""
    run_command(["sh", ISO_SCRIPT])

def run_os():
    """Run the OS with the provided script"""
    run_command(["sh", RUN_SCRIPT])

def clean():
    """Clean up the build directory"""
    if not os.path.exists(OBJ_DIR):
        print(f"Directory '{OBJ_DIR}' does not exist.")
        return
    
    if os.path.exists(OBJ_DIR):
        # List all items (files and directories) in the directory
        for item in os.listdir(OBJ_DIR):
            item_path = os.path.join(OBJ_DIR, item)
            
            # If it's a directory, remove its contents recursively
            if os.path.isdir(item_path):
                for sub_item in os.listdir(item_path):
                    sub_item_path = os.path.join(item_path, sub_item)
                    if os.path.isdir(sub_item_path):
                        os.rmdir(sub_item_path)  # Remove empty subdirectory
                    else:
                        os.remove(sub_item_path)  # Remove file
                os.rmdir(item_path)  # Remove the empty directory
            else:
                os.remove(item_path)  # If it's a file, remove it

        print(f"Directory '{OBJ_DIR}' has been cleaned.")
    else:
        print(f"Directory '{OBJ_DIR}' does not exist.")
        
    if os.path.exists(TARGET):
        os.remove(TARGET)


def main():
    global ASM_SOURCES, C_SOURCES, ASM_OBJECTS, C_OBJECTS
    if len(sys.argv) < 2:
        print("Usage: make.py [all|iso|run|clean]")
        sys.exit(1)

    command = sys.argv[1]
    
    # Get all source files (in subdirectories)
    asm_sources, c_sources = get_sources()

    # Create object file paths based on source files
    ASM_SOURCES = [_ for _ in asm_sources]
    C_SOURCES = [_ for _ in c_sources]

    move_element(ASM_SOURCES, os.path.abspath("./asm/grub_entry.asm"), 0)
    move_element(C_SOURCES, os.path.abspath("./src/core/kernel.c"), 0)
            
    ASM_OBJECTS = [os.path.join(OBJ_DIR, f.split('/')[-1].replace('.asm', '.asm.o')) for f in ASM_SOURCES]
    C_OBJECTS = [os.path.join(OBJ_DIR, f.split('/')[-1].replace('.c', '.c.o')) for f in C_SOURCES]

    with open("log.log", 'w') as log:
        log.write(str(ASM_SOURCES))
        log.write(str(ASM_OBJECTS))
        log.write(str(C_SOURCES))
        log.write(str(C_OBJECTS))

    if command == 'all':
        os.makedirs(OBJ_DIR, exist_ok=True)
        build_asm()
        build_c()
        link()
    elif command == 'iso':
        create_iso()
    elif command == 'run':
        run_os()
    elif command == 'clean':
        clean()
    elif command == 'help' or command == '-help' or command == '--help' \
    or command == 'h' or command == '-h' or command == '--h':
        print("Usage: make.py [all|iso|run|clean]")
    else:
        print("Unknown command:", command)
        sys.exit(1)


if __name__ == '__main__':
    main()