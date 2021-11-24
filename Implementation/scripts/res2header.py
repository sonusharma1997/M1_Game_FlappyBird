import argparse
import io
from itertools import zip_longest
import logging
import os
import sys

from PIL import Image
import pywavefront
pywavefront.configure_logging(logging.CRITICAL)

# Requirements:
# pillow
# pywavefront


def grouper(iterable, n, fillvalue=None):
    "Collect data into fixed-length chunks or blocks"
    # grouper('ABCDEFG', 3, 'x') --> ABC DEF Gxx"
    args = [iter(iterable)] * n
    return zip_longest(*args, fillvalue=fillvalue)


def model2header(resource_file):
    name, ext = os.path.splitext(os.path.basename(resource_file))
    format = ''

    vertices = []
    model = pywavefront.Wavefront(resource_file)
    for _, material in model.materials.items():
        format = material.vertex_format
        for vertex in material.vertices:
            vertices.append(vertex)

    vertex_size = 0
    if format == 'V3F':
        format = 'MODEL_FORMAT_V3F'
        vertex_size = 3
    elif format == 'T2F_V3F':
        format = 'MODEL_FORMAT_T2F_V3F'
        vertex_size = 5
    elif format == 'N3F_V3F':
        format = 'MODEL_FORMAT_N3F_V3F'
        vertex_size = 6
    elif format == 'T2F_N3F_V3F':
        format = 'MODEL_FORMAT_T2F_N3F_V3F'
        vertex_size = 8
    else:
        raise SystemExit('Unknown model format: {}'.format(format))

    count = len(vertices) // vertex_size
    guard = 'MODELS_{}_H_INCLUDED'.format(name.upper())

    s = io.StringIO()
    s.write('// THIS FILE WAS AUTOGENERATED BY:\n')
    s.write('// python3 ' + ' '.join(sys.argv) + '\n')
    s.write('#ifndef {}\n'.format(guard))
    s.write('#define {}\n'.format(guard))
    s.write('\n')
    s.write('#include "model.h"\n')
    s.write('\n')
    s.write('static const char MODEL_{}_PATH[] = "{}";\n'.format(name.upper(), resource_file))
    s.write('static const int MODEL_{}_FORMAT = {};\n'.format(name.upper(), format))
    s.write('static const long MODEL_{}_VERTEX_COUNT = {};\n'.format(name.upper(), count))
    s.write('static const float MODEL_{}_VERTICES[] = {{\n'.format(name.upper()))
    for group in grouper(vertices, vertex_size):
        group = list(group)
        while None in group:
            group.remove(None)
        line = ', '.join('{: f}f'.format(b) for b in group)
        s.write('    {},\n'.format(line))
    s.write('};\n')
    s.write('\n')
    s.write('#endif\n')

    return s.getvalue()


def shader2header(resource_file):
    name, ext = os.path.splitext(os.path.basename(resource_file))
    with open(resource_file) as f:
        source = f.read()

    guard = 'SHADERS_{}_H_INCLUDED'.format(name.upper())

    s = io.StringIO()
    s.write('// THIS FILE WAS AUTOGENERATED BY:\n')
    s.write('// python3 ' + ' '.join(sys.argv) + '\n')
    s.write('#ifndef {}\n'.format(guard))
    s.write('#define {}\n'.format(guard))
    s.write('\n')
    s.write('#include "shader.h"\n')
    s.write('\n')
    s.write('static const char SHADER_{}_PATH[] = "{}";\n'.format(name.upper(), resource_file))
    s.write('static const char SHADER_{}_SOURCE[] = \n'.format(name.upper()))
    for line in source.splitlines():
        s.write('    "{}\\n"\n'.format(line))
    s.write(';\n')
    s.write('\n')
    s.write('#endif\n')

    return s.getvalue()


def texture2header(resource_file):
    name, ext = os.path.splitext(os.path.basename(resource_file))

    # load image and flip vertically to accommodate OpenGL's texcoord system
    texture = Image.open(resource_file)
    texture = texture.transpose(Image.FLIP_TOP_BOTTOM)

    row_size = 0
    format = texture.mode
    if format == 'RGB':
        row_size = 12
        format = 'TEXTURE_FORMAT_RGB'
    elif format == 'RGBA':
        row_size = 16
        format = 'TEXTURE_FORMAT_RGBA'
    else:
        raise SystemExit('Unknown texture format: {}'.format(format))

    width, height = texture.size
    pixels = texture.tobytes()

    guard = 'TEXTURES_{}_H_INCLUDED'.format(name.upper())

    s = io.StringIO()
    s.write('// THIS FILE WAS AUTOGENERATED BY:\n')
    s.write('// python3 ' + ' '.join(sys.argv) + '\n')
    s.write('#ifndef {}\n'.format(guard))
    s.write('#define {}\n'.format(guard))
    s.write('\n')
    s.write('#include "texture.h"\n')
    s.write('\n')
    s.write('static const char TEXTURE_{}_PATH[] = "{}";\n'.format(name.upper(), resource_file))
    s.write('static const int TEXTURE_{}_FORMAT = {};\n'.format(name.upper(), format))
    s.write('static const long TEXTURE_{}_WIDTH = {};\n'.format(name.upper(), width))
    s.write('static const long TEXTURE_{}_HEIGHT = {};\n'.format(name.upper(), height))
    s.write('static const unsigned char TEXTURE_{}_PIXELS[] = {{\n'.format(name.upper()))
    for group in grouper(pixels, row_size):
        group = list(group)
        while None in group:
            group.remove(None)
        line = ', '.join('0x{:02x}'.format(b) for b in group)
        s.write('    {},\n'.format(line))
    s.write('};\n')
    s.write('\n')
    s.write('#endif\n')

    return s.getvalue()


def res2header(resource_file):
    _, ext = os.path.splitext(os.path.basename(resource_file))
    if ext in ['.obj']:
        return model2header(resource_file)
    elif ext in ['.glsl']:
        return shader2header(resource_file)
    elif ext in ['.jpg', '.png']:
        return texture2header(resource_file)
    else:
        raise SystemExit('Unknown resource type: {}'.format(resource_file))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert game resources into C headers')
    parser.add_argument('resource_file', help='input resource file')
    parser.add_argument('header_file', help='output header file')
    args = parser.parse_args()

    header = res2header(args.resource_file)
    with open(args.header_file, 'w') as f:
        f.write(header)
