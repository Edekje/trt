import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import colors
import numpy as np
import sys
from amrplot import read
from sys import argv

# --- PARAMETER SETTINGS ---

timepersec = 2

lines = open(sys.argv[1], 'r').readlines()
if lines[4] != "# mode: grid\n":
    print("Incorrectly formatted input file.")
    exit()

shocked_cutoff = float(lines[6][:-1].strip())
if len(argv)>=3:
    data_filename = argv[2]
else:
    data_filename = lines[1][13:-1]

# --- READ INPUT ---

l = lines[3][:-1].split(sep=', ')
slice_start_num = int(l[0])
slice_stop_num = int(l[1])
# slice_stop_num = 101 # temporary f/ debug
slice_start_time = float(l[2])
slice_timestep = float(l[3])
max_radius = float(l[4])

l = lines[8][:-1].split(sep=', ')
tobs_start = float(l[0])
tobs_stop = float(l[1])
# tobs_stop = 20 # temporary f/ debug
tobs_step = float(l[2])
a_start = float(l[3])
a_stop = float(l[4])
a_step = float(l[5])

tobs = np.arange(tobs_start, tobs_stop, tobs_step)
a = np.arange(a_start, a_stop, a_step)
slices= np.arange(slice_start_num, slice_stop_num)

if tobs_step==slice_timestep and tobs_start==slice_start_time and len(slices)>=len(tobs):
    graph_fluid_prop = True
    data = []
    for i in slices:
        d=read.load(i,file=data_filename,type='vtu')
        d.getPointData()
        pd = d.pointdata.GetPointData()
        N = pd.GetNumberOfTuples()
        d.r = np.array([pd.GetArray('r').GetComponent(i,0) for i in range(N)])
        d.rho = np.array([pd.GetArray('rho').GetComponent(i,0) for i in range(N)])
        d.p = np.array([pd.GetArray('p').GetComponent(i,0) for i in range(N)])
        d.u1 = np.array([pd.GetArray('u1').GetComponent(i,0) for i in range(N)])
        d.gammaeff = np.array([pd.GetArray('gammaeff').GetComponent(i,0) for i in range(N)])
        d.e_th = d.p / (d.gammaeff-1)

        data.append(d)
else:
    graph_fluid_prop = False

l = lines[10][:-1].split(sep=', ')
frequency = float(l[2])

I = [ float(l[:-1].split(sep=', ')[4]) for l in lines[10:10+len(tobs)*len(a)] ]
minfloor = 1e-40
temp = [x for x in I if x > minfloor] # weed out numbers below tiny thresh
temp.sort()
minfloor = temp[len(temp)*5//1000] # find out val of 0.5% lowest number
Iavg = np.average(temp)

# For use in normalising the intensity graph:
Inorm = 10**(-round(np.log10(Iavg)))
#Inorm = 1e13 or set your own....

I = np.array([ max(x,minfloor) for x in I])
tau = np.array([ float(l[:-1].split(sep=', ')[3]) for l in lines[10:10+len(tobs)*len(a)] ])
I = I.reshape((len(tobs), len(a)))
tau = tau.reshape((len(tobs), len(a)))

print("Max optimal depth:", np.max(tau))

# --- CONFIGURE ANIMATION ---

frames = range(len(tobs))
"""
fig,axes = plt.subplots(1,2)
fig.set_size_inches(11.0,5)
image_ax = axes[0]
ax = axes[1]
"""
fig = plt.figure()
fig.set_size_inches(16.0,5)
ax = fig.add_subplot(131)
im_ax = fig.add_subplot(132, projection='polar')
im_ax = fig.add_subplot(132, projection='polar')
ax3 = fig.add_subplot(133)

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
# im_ax.set_xticklabels([]) # does not work
im_ax.set_yticklabels([])
fig.colorbar(h, label='$I_{\\nu}$', ax=ax3)

# GRAPH

line, = ax.plot(a, I[0]*Inorm, label='$I \cdot I_{norm}$')

if graph_fluid_prop:
    line_2, = ax.plot(data[0].r,data[0].rho, label='$\\rho$')
    line_3, = ax.plot(data[0].r,data[0].u1, label='$u_{1}$')
    line_4, = ax.plot(data[0].r,data[0].e_th/data[0].rho, label='$e_{th}/\\rho$')
    line_5, = ax.plot(data[0].r,data[0].p, label='$p$')

ax.set_yscale('log')
ax.set_xlabel('a $\\rightarrow$')
ax.set_ylabel('A.U. $\\rightarrow$')
ax.axhline(shocked_cutoff, label='Shocked Gas Cutoff', linestyle='--', color='k')
ax.set_xlim(a[0], a[-1])
ax.set_ylim(Imin*Inorm, Imax*Inorm)
axtext = fig.add_axes([0.0, 0.95, 0.1, 0.05])
axtext.axis('off')

# FREQ PLOT

F = np.trapz(I*a*2*np.pi, x=a, axis=1) # needs circular integration
Fmin = np.min(F)
Fmax=np.max(F)
line3, = ax3.plot(tobs[0:], F[0:])
ax3.set_yscale('log')
ax3.set_xlabel('a $\\rightarrow$')
ax3.set_xlabel('$t_{obs}$ $\\rightarrow$')
ax3.set_ylabel('log($F_{\\nu}$) $\\rightarrow$')
ax3.set_xlim(np.min(tobs), np.max(tobs))
ax3.set_ylim(Fmin, Fmax)
fig.legend()

# fig.tight_layout()
# --- UPDATE FRAME ---

shocked_regions = []

def make_frame(i):
    global shocked_regions
    if i < 2:
        ax.relim()
        ax.autoscale_view()
    # Image
    z = np.tile(I[i], (r.shape[0],1))
    h.set_array(z.ravel())

    # Graph
    line.set_data(a, I[i]*Inorm)
    line_2.set_data(data[i].r,data[i].rho)
    line_3.set_data(data[i].r,data[i].u1)
    gammaf = data[i].e_th/data[i].rho
    line_4.set_data(data[i].r,gammaf)
    line_5.set_data(data[i].r,data[i].p)
    # line.set_data(a, np.log(I[i]))

    time = axtext.text(0.5, 0.5, "$I_{norm}=%.1e$, $t=%.2f$"%(Inorm, tobs[i]), ha='left', va='top')
    
    line3.set_data(tobs[:i], F[:i])
    
    # These must always be drawn:
    standard = [line, line_2, line_3, line_4, line_5, line3, time]
    # Remove shocked regions:
    for reg in shocked_regions:
        reg.remove()
    shocked = False
    shocked_regions = []
    # Identify shocked regions
    temp_gf = np.concatenate([[0],np.where(gammaf>shocked_cutoff, 1, 0),[0]])
    boundaries = data[i].r[np.nonzero(temp_gf[1:]!=temp_gf[:-1])]
    # Redraw shocked regions
    for sta, sto in zip(boundaries[::2],boundaries[1::2]):
        shocked_regions.append(ax.axvspan(sta,sto, alpha=0.1, color='red', zorder=-1)) # zorder=-1 -> draw behind
    #fig.canvas.draw()

    return standard+shocked_regions

# --- RUN ANIMATION ---

animation = animation.FuncAnimation(fig, make_frame, frames=frames, interval=tobs_step/timepersec*1000, blit=True, save_count=50)
# interval is ms per frame

#animation.save('video.mp4') # save video

plt.show()
