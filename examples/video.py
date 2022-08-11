import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import colors
import numpy as np
import sys

# --- READ INPUT ---

lines = open(sys.argv[1], 'r').readlines()
if lines[4] != "# mode: grid\n":
    print("Incorrectly formatted input file.")
    exit()

l = lines[3][:-1].split(sep=', ')
slice_start_num = int(l[0])
slice_stop_num = int(l[1])
slice_start_time = float(l[2])
slice_timestep = float(l[3])
max_radius = float(l[4])

l = lines[6][:-1].split(sep=', ')
tobs_start = float(l[0])
tobs_stop = float(l[1])
tobs_step = float(l[2])
a_start = float(l[3])
a_stop = float(l[4])
a_step = float(l[5])

l = lines[8][:-1].split(sep=', ')
frequency = float(l[2])

tobs = np.arange(tobs_start, tobs_stop, tobs_step)
a = np.arange(a_start, a_stop, a_step)

I = np.array([ float(l[:-1].split(sep=', ')[4]) for l in lines[8:] ])
tau = np.array([ float(l[:-1].split(sep=', ')[3]) for l in lines[8:] ])
I = I.reshape((len(tobs), len(a)))
tau = tau.reshape((len(tobs), len(a)))

print(np.max(I))

# --- CONFIGURE ANIMATION ---

frames = range(len(tobs))
"""
fig,axes = plt.subplots(1,2)
fig.set_size_inches(11.0,5)
image_ax = axes[0]
ax = axes[1]
"""
fig = plt.figure()
fig.set_size_inches(11.0,5)
ax = fig.add_subplot(121)
im_ax = fig.add_subplot(122, projection='polar')

temp = I[~np.isnan(I)]
Imin = np.min(temp)
Imax = np.max(temp)

# IMAGE

colorscheme = 'hot'
# (https://matplotlib.org/stable/tutorials/colors/colormaps.html)

azm = np.linspace(0, 2*np.pi, 1000)
r, th = np.meshgrid(a,azm)
z = np.tile(I[0], (r.shape[0],1))
im_ax.grid(False)
h = im_ax.pcolormesh(th, r, z, norm=colors.LogNorm(Imin, Imax), cmap=plt.get_cmap(colorscheme))
im_ax.plot(azm, r, ls='none')
im_ax.set_xticklabels([])
fig.colorbar(h, label='$I_{\\nu}$')

# GRAPH

line, = ax.plot(a, np.log(I[0]))
ax.set_xlabel('a $\\rightarrow$')
ax.set_ylabel('log($I$) $\\rightarrow$')
ax.autoscale_view()
ax.set_ylim(np.log(Imin)*1.05, np.log(Imax)*1.05)
axtext = fig.add_axes([0.0, 0.95, 0.1, 0.05])
axtext.axis('off')

# --- UPDATE FRAME ---

def make_frame(i):
    if i < 2:
        ax.relim()
        ax.autoscale_view()
    # Image
    z = np.tile(I[i], (r.shape[0],1))
    h.set_array(z.ravel())

    # Graph
    line.set_data(a, np.log(I[i]))

    time = axtext.text(0.5, 0.5, str(i), ha='left', va='top')
    
    fig.canvas.draw()

    return line, time, 

# --- RUN ANIMATION ---

animation = animation.FuncAnimation(fig, make_frame, frames=frames, interval=100, blit=True, save_count=50)
# interval is ms per frame

plt.show()
