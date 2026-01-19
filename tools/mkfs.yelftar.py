#!/bin/python3

import argparse
import collections
from collections.abc import Mapping
import math
from pathlib import Path
from dataclasses import dataclass
    

@dataclass
class Node:
    # parent: 'Node' | None
    name: str
    data: bytes | list['Node'] # None means it's a directory

    def __hash__(self):
        return id(self)


def calculate_file_block_size(data_size: int) -> int:
    """Returns blocks count needed for a file."""

    if data_size == 0:
        return 1
        
    space_in_first_block = 512 - 72
    
    if data_size <= space_in_first_block:
        return 1
        
    remaining_data = data_size - space_in_first_block
    additional_blocks = math.ceil(remaining_data / 512)
    
    return 1 + additional_blocks


def scan_directory(this_node: Node, dir_path: Path):
    for path in dir_path.iterdir():
        child_node = Node(
            path.name, 
            path.read_bytes() if path.is_file() else [],
        )
        this_node.data.append(child_node)
        if path.is_dir():
            scan_directory(child_node, path)


def hie_to_flat(node: Node, nodes_list: list[Node], lba_sizes_list: list[int]):
    if isinstance(node.data, bytes): # is a file
        nodes_list.append(node)
        lba_sizes_list.append(calculate_file_block_size(len(node.data)))
        return
    
    nodes_list.append(node)
    lba_sizes_list.append(calculate_file_block_size(len(node.data) * 4))
    
    for child in node.data:
        hie_to_flat(child, nodes_list, lba_sizes_list)


def compose_filesystem(root: Path, out: Path):
    hierarchy = Node('', [])
    scan_directory(hierarchy, root)

    nodes_list: list[Node] = []
    lba_sizes_list: list[int] = []
    hie_to_flat(hierarchy, nodes_list, lba_sizes_list)

    nodes_lbas_dict: Mapping[Node, int] = collections.OrderedDict()
    current_lba = 1
    for node, lba_size in zip(nodes_list, lba_sizes_list):
        nodes_lbas_dict[node] = current_lba
        current_lba += lba_size

    with out.open('wb') as file:
        file.write(b'yelftar.\x01\x00\x00\x00')
        file.write(b'\x00' * (512 - 12))

        for node, lba_size in zip(nodes_list, lba_sizes_list):
            # name
            file.write(node.name.encode('UTF-8')[:63].ljust(64, b'\x00'))
            # size
            size = len(node.data)
            if isinstance(node.data, list):
                size *= 4
            file.write(size.to_bytes(4, 'little'))
            # type
            file.write(b'\x02' if isinstance(node.data, list) else b'\x01') # 2=dir 1=file
            # padding
            file.write(b'\x00\x00\x00')

            if isinstance(node.data, bytes): # is a file
                file.write(node.data)
            else:
                for child in node.data:
                    child_lba = nodes_lbas_dict[child]
                    file.write(child_lba.to_bytes(4, 'little'))
            
            size_with_hdr = size + 72
            file.write(b'\x00' * (512 - size_with_hdr % 512))


def main():
    p = argparse.ArgumentParser(
        description='Create a yelf-tar v1 filesystem image.'
    )
    p.add_argument('root', help='Input filesystem root directory')
    p.add_argument('output', help='Output image file')
    args = p.parse_args()

    root_path = Path(args.root)
    out_path = Path(args.output)

    compose_filesystem(root_path, out_path)


if __name__ == '__main__':
    main()
