# Note

This is a hacky fork of HokakuCTR made to get packet dumps from Sonic Generations on the 3DS.  Credit goes to Pretendo for originally creating this program.
Main things of note are as follows:

1. All dumps will be saved at /HokakuCTR/SonicGen - (Title ID)/
2. All dumps are named "dump.pcap".  You will need to rename the dump for each dump you do so they don't get overridden.  I recommend installing 3DShell to speed this process up.
3. Due to this being a hacky workaround, once you close Sonic Generations and open another app, the 3DS will crash (at least in my testing anyway).  I recommend opening the Rosalina menu, disabling the plugin, then selecting "Reboot" to get around this.
4. Use this for Sonic Generations only.  You could technically use this for other games, but I wouldn't recommend it.

# HokakuCTR 

A 3DS game plugin (3GX) to dump the RMC communication between 3DS games and NEX. The traffic is dumped to the SD into **pcap** files.

## Usage

1. Install the [Luma3DS 3GX plugin loader](https://github.com/Nanquitas/Luma3DS/releases/latest).
2. Navigate to **luma/plugins** and copy the **.3gx** file as **default.3gx** to load it for all game or place it inside a folder with the game *titleID* of the game you want to use.
3. Open the Rosalina menu and enable the plugin loader.
4. Launch the game to dump traffic from. The screen should flash blue and some text will display on the screen. If it says **Not Ready**, it means this game is not compatible. If it says **Ready**, you can go online to dump the traffic.

The dumps will be placed inside the **HokakuCTR** folder in the SD card root.
