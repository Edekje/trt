import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import colors
import numpy as np
import sys
from amrplot import read
from sys import argv

# --- PARAMETER SETTINGS ---

blitinterval = 40

UNIT=1e-6*1e-23
UNITNAME='$\\mu$Jy'

lines = open(sys.argv[1], 'r').readlines()

shocked_cutoff = float(lines[5][:-1].split(sep=', ')[0])
if len(argv)>=3:
    data_filename = argv[2]
else:
    data_filename = lines[1][13:-1]

distance = 40.4*1e6*3.086e18 # distance in Mpc

# --- READ INPUT ---

l = lines[3][:-1].split(sep=', ')
slice_start_num = int(l[0])
slice_stop_num = int(l[1])
# slice_stop_num = 101 # temporary f/ debug
slice_start_time = float(l[2])
slice_timestep = float(l[3])
max_radius = float(l[4])
M = float(lines[5][:-1].split(sep=', ')[1])
L = float(lines[5][:-1].split(sep=', ')[2])

l = lines[9][:-1].split(sep=', ')
if lines[8]!='# tobs:\n':
    tobs_start = float(l[0])
    tobs_stop = float(l[1])
    # tobs_stop = 20 # temporary f/ debug
    tobs_step = float(l[2])
    tobs = np.arange(tobs_start, tobs_stop, tobs_step)
else:
    tobs = np.array([float(f) for f in l])

l = lines[11][:-1].split(sep=', ')
if lines[10]!='# a:\n':
    a_start = float(l[0])
    a_stop = float(l[1])
    a_step = float(l[2])
    a = np.arange(a_start, a_stop, a_step)
else:
    a = np.array([float(f) for f in l])

slices= np.arange(slice_start_num, slice_stop_num)

if lines[8]!='# tobs:\n' and tobs_step==slice_timestep and tobs_start==slice_start_time and len(slices)>=len(tobs):
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

l = lines[13][:-1].split(sep=', ')
frequencies = [float(f) for f in lines[7][:-1].split(sep=', ')]
frequency = frequencies[0]

I = [ float(l[:-1].split(sep=', ')[4]) for l in lines[13:13+len(tobs)*len(a)*len(frequencies)] ]
minfloor = 1e-40
temp = [x for x in I if x > minfloor] # weed out numbers below tiny thresh
temp.sort()
minfloor = temp[len(temp)*5//1000] # find out val of 0.5% lowest number
minfloor = temp[len(temp)*0//1000] # find out val of 0.5% lowest number
Iavg = np.average(temp)


I = np.array([ max(x,minfloor) for x in I])

temp = I[~np.isnan(I)]
Imin = np.min(temp)
Imax = np.max(temp)
# For use in normalising the intensity graph:
#Inorm = 10**(-round(np.log10(Iavg)))
Inorm = 1/np.sqrt(Imin*Imax) # geom mean
#Inorm = 1e13 or set your own....

I_list = I.reshape((len(tobs), len(a), len(frequencies)))

I_list = I_list[7:]
tobs = tobs[7:]

I = I_list[:,:,0]

tau = np.array([ float(l[:-1].split(sep=', ')[3]) for l in lines[13:13+len(tobs)*len(a)*len(frequencies)] ])
tau_list = tau.reshape((len(tobs), len(a), len(frequencies)))
tau = tau_list[:,:,0]

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
fig.set_size_inches(14.0,6)
ax = fig.add_subplot(131)
im_ax = fig.add_subplot(132, projection='polar')
im_ax = fig.add_subplot(132, projection='polar')
ax3 = fig.add_subplot(133)

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

I_plots = []

for i in range(len(frequencies)):
#    line, = ax.plot(a, I_list[0,:,i]*Inorm, label='$I \cdot I_{norm} \\nu=%.1e$'%(frequencies[i]))
    line, = ax.plot(a, I_list[0,:,i], label='$I \cdot I_{norm} \\nu=%.1e$'%(frequencies[i]))
    I_plots.append(line)

if graph_fluid_prop:
    line_2, = ax.plot(data[0].r,data[0].rho, label='$\\rho$')
    line_3, = ax.plot(data[0].r,data[0].u1, label='$u_{1}$')
    line_4, = ax.plot(data[0].r,data[0].e_th/data[0].rho, label='$e_{th}/\\rho$')
    line_5, = ax.plot(data[0].r,data[0].p, label='$p$')

ax.set_yscale('log')
ax.set_xlabel('a $\\rightarrow$')
#ax.set_ylabel('A.U. $\\rightarrow$')
ax.set_ylabel('$I_{\\nu}\\ \\rightarrow$')
#ax.axhline(shocked_cutoff, label='Shocked Gas Cutoff', linestyle='--', color='k')
ax.set_xlim(a[0], a[-1])
ax.set_ylim(Imin*Inorm, Imax*Inorm)
axtext = fig.add_axes([0.0, 0.95, 0.35, 0.08])
axtext.axis('off')
time = axtext.text(0.5, 0.5, "$I_{norm}=%.1e$, $t=%.2f$"%(Inorm, tobs[0]), ha='left', va='top')

# FREQ PLOT

F_list = []

for i in range(len(frequencies)):
    # integrated specific intensity in one direction
    F = np.trapz(I_list[:,:,i]*a*2*np.pi, x=a, axis=1)*L**2 # needs circular integration
    # Total luminosity
    F *= 4 * np.pi
    # Flux
    F /= 4 * np.pi * distance**2
    # Unit
    F /= UNIT
    F_list.append(F)
F_list = np.array(F_list)
F = F_list[0]

Fmin = np.min(F_list)
Fmax=np.max(F_list)

F_plots = []
for i in range(len(frequencies)):
    F_plots.append(ax3.plot(tobs[0:], F_list[i,0:], label='F $\\nu=%.1e$'%(frequencies[i]))[0])

ax3.set_yscale('log')
ax3.set_xscale('log')
ax3.set_xlabel('a $\\rightarrow$')
ax3.set_xlabel('$t_{obs}$ $\\rightarrow$')
ax3.set_ylabel('$F_{\\nu}$ ('+UNITNAME+')        $\\rightarrow$')
#ax3.set_xlim(np.min(tobs), np.max(tobs))
ax3.set_xlim(30, np.max(tobs))
#ax3.set_ylim(Fmin, Fmax)
ax3.set_ylim(0.3, 200)
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
    for j in range(len(frequencies)):
        I_plots[j].set_data(a,I_list[i,:,j]*Inorm)
    
    time.set_text("$I_{norm}=%.1e$, $t=%.2f$"%(Inorm, tobs[i]))
    standard = [h, time]
    
    if graph_fluid_prop:
        line_2.set_data(data[i].r,data[i].rho)
        line_3.set_data(data[i].r,data[i].u1)
        gammaf = data[i].e_th/data[i].rho
        line_4.set_data(data[i].r,gammaf)
        line_5.set_data(data[i].r,data[i].p)
        # line.set_data(a, np.log(I[i]))
        standard.extend([line_2, line_3, line_4, line_5])
    
    # Fluxes    
    for j in range(len(frequencies)):
        F_plots[j].set_data(tobs[:i], F_list[j,:i])
    
    # These must always be drawn:
    if graph_fluid_prop:
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

    return standard+shocked_regions+F_plots+I_plots

# --- RUN ANIMATION ---

animation = animation.FuncAnimation(fig, make_frame, frames=frames, interval=blitinterval, blit=True, save_count=50)
# interval is ms per frame

animation.save('PRESENT.mp4') # save video

plt.show()
