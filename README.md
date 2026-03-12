# CIF Crystal Structure Visualizer

A real-time 3D crystal structure visualization tool written in C++ with OpenGL. Load crystallographic data from standard `.cif` files and interactively explore atomic arrangements and simulated X-ray diffraction (XRD) reciprocal space patterns.

---

## Features

- **CIF File Parsing** — Reads standard Crystallographic Information Files (`.cif`), extracting lattice parameters, fractional atom coordinates, and symmetry operations
- **Symmetry Expansion** — Automatically applies space group symmetry operations to generate the full unit cell from the asymmetric unit, with duplicate detection and unit-cell wrapping
- **3D Atom Rendering** — Instanced billboard rendering of atoms with element-accurate colors and covalent radii (CPK-style coloring for H, C, N, O, Fe, Cu, Zn, Al, Si, Ti, Ni, Cr, Mg, Ca, Na)
- **Unit Cell Wireframe** — Toggleable bounding box showing the edges of the crystallographic unit cell
- **Reciprocal Space / XRD View** — Toggle between the real-space crystal structure and a simulated XRD reciprocal lattice point pattern
- **Interactive Camera** — Orbit, zoom, and pan the structure with mouse drag and scroll
- **Dynamic File Loading** — Open any `.cif` file at runtime via a native OS file dialog, with full GPU resource cleanup and reinitialization
- **On-screen Element Key** — ImGui overlay displaying a color-coded legend of all elements present in the loaded structure

---

## Dependencies

| Library | Purpose |
|---|---|
| [OpenGL 4.6](https://www.opengl.org/) | GPU rendering |
| [GLEW](https://glew.sourceforge.net/) | OpenGL extension loading |
| [GLFW](https://www.glfw.org/) | Window creation and input handling |
| [GLM](https://github.com/g-truc/glm) | Mathematics (vectors, matrices, transforms) |
| [Dear ImGui](https://github.com/ocornut/imgui) | Immediate-mode GUI overlay |
| [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) | Native OS file open dialog |

---

## Building

### Prerequisites

- A C++17 compatible compiler (MSVC, GCC, Clang)
- OpenGL 4.6-capable GPU and drivers
- The dependency libraries listed above installed and linked

### Visual Studio (Windows)

1. Clone the repository:
2. Open the solution or create a new Visual Studio C++ project and add all source files.
3. Configure include and library paths for GLEW, GLFW, GLM, and ImGui in project properties.
4. Link against `opengl32.lib`, `glew32.lib`, `glfw3.lib`.
5. Build in **Release** or **Debug** mode.

> Ensure all ImGui source files are included in the build. GLM is header-only and requires no linking.

---

## Usage

### Running

Place the executable in the same directory as the GLSL shader files:

```
atoms.vert / atoms.frag
points.vert / points.frag
wireframe.vert / wireframe.frag
```
### Controls

| Input | Action |
|---|---|
| **Left Mouse Drag** | Orbit camera (azimuth and elevation) |
| **Scroll Wheel** | Zoom in / out |
| **Space** | Toggle between real-space atom view and reciprocal-space XRD view |
| **W** | Toggle unit cell wireframe on/off |
| **O** | Open a `.cif` file via native file dialog |

---

## Project Structure

```
├── Source.cpp              # Main application: OpenGL init, render loop, input callbacks
├── CIFParser.h / .cpp      # CIF file parser: lattice, atom sites, symmetry operations
├── XRDPoints.h / .cpp      # XRD reciprocal lattice point generation
├── AtomData.h              # Element color and radius lookup tables
├── atoms.vert / .frag      # Instanced billboard atom shaders
├── points.vert / .frag     # XRD point shaders (intensity-scaled)
├── wireframe.vert / .frag  # Unit cell wireframe shaders
├── tinyfiledialogs.h / .cpp# Cross-platform native file dialog
└── imgui.ini               # ImGui layout persistence
```

---

## How It Works

### CIF Parsing

`CIFParser` reads the CIF file line by line, extracting:
- **Lattice parameters** (`_cell_length_a/b/c`, `_cell_angle_alpha/beta/gamma`)
- **Symmetry operations** from the `_space_group_symop_operation_xyz` block, parsed into 3×3 rotation matrices and translation vectors
- **Atom sites** from the `_atom_site_label`, `_atom_site_fract_x/y/z` columns

After parsing, `applySymmetry()` generates all symmetry-equivalent positions within the unit cell, wraps coordinates to [0,1), and removes duplicates within a 0.001 Å tolerance.

### Atom Rendering

Atoms are rendered using **instanced billboard quads** — each atom is a screen-aligned quad shaded in the fragment shader to appear as a sphere using the offset from the quad center. This avoids the cost of per-atom geometry while producing smooth, depth-correct spheres. Atoms that fall on a unit cell face (fractional coordinate near 0) are replicated across all adjacent cell corners.

### Reciprocal Space

The XRD view generates a set of reciprocal lattice points (`XRDPoints::genFakeXRD`) scaled by the lattice parameter `a`, rendered as GL_POINTS with intensity-modulated size and color via the point shader.

---

## Supported Elements

The following elements have defined colors and covalent radii. Unrecognized elements render as gray spheres with a default radius of 1.0 Å.

| Element | Color |
|---|---|
| H | White |
| C | Dark gray |
| N | Blue |
| O | Red |
| Fe | Orange-brown |
| Cu | Copper orange |
| Zn | Blue-gray |
| Al | Green |
| Si | Brown |
| Ti | Cool gray |
| Ni | Teal |
| Cr | Dark teal |
| Mg | Light gray |
| Ca | Soft green |
| Na | Purple |

---

## Roadmap / Known Limitations

- The font path for ImGui is hardcoded to `C:\Windows\Fonts\arial.ttf`; cross-platform font loading is planned
- Lattice angle parameters (`alpha`, `beta`, `gamma`) are parsed but not yet used to construct non-orthogonal unit cells
- Only a single unit cell is visualized; supercell expansion is not yet supported

---

## Acknowledgements

- [tinyfiledialogs](http://tinyfiledialogs.sourceforge.net/) by Guillaume Vareille
- [Dear ImGui](https://github.com/ocornut/imgui) by Omar Cornut
- Crystallographic data format: [IUCr CIF standard](https://www.iucr.org/resources/cif)
