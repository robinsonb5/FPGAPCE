library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use IEEE.numeric_std.ALL;

library altera;
use altera.altera_syn_attributes.all;

library work;


entity C3BoardToplevel is
port(
		clk_50 	: in 	std_logic;
		reset_n : in 	std_logic;
		led_out : out 	std_logic;
		btn1 : in std_logic;
		btn2 : in std_logic;

		-- SDRAM - chip 1
		sdram1_clk : out std_logic; -- Different name format to escape wildcard in SDC file
		sd1_addr : out std_logic_vector(11 downto 0);
		sd1_data : inout std_logic_vector(7 downto 0);
		sd1_ba : out std_logic_vector(1 downto 0);
		sd1_cke : out std_logic;
		sd1_dqm : out std_logic;
		sd1_cs : out std_logic;
		sd1_we : out std_logic;
		sd1_cas : out std_logic;
		sd1_ras : out std_logic;

		-- SDRAM - chip 2
		sdram2_clk : out std_logic; -- Different name format to escape wildcard in SDC file
		sd2_addr : out std_logic_vector(11 downto 0);
		sd2_data : inout std_logic_vector(7 downto 0);
		sd2_ba : out std_logic_vector(1 downto 0);
		sd2_cke : out std_logic;
		sd2_dqm : out std_logic;
		sd2_cs : out std_logic;
		sd2_we : out std_logic;
		sd2_cas : out std_logic;
		sd2_ras : out std_logic;
		
		-- VGA
		vga_red 		: out unsigned(5 downto 0);
		vga_green 	: out unsigned(5 downto 0);
		vga_blue 	: out unsigned(5 downto 0);
		
		vga_hsync 	: buffer std_logic;
		vga_vsync 	: buffer std_logic;

		-- PS/2
		ps2k_clk : inout std_logic;
		ps2k_dat : inout std_logic;
		ps2m_clk : inout std_logic;
		ps2m_dat : inout std_logic;
		
		-- Audio
		aud_l : out std_logic;
		aud_r : out std_logic;
		
		-- RS232
		rs232_rxd : in std_logic;
		rs232_txd : out std_logic;

		-- SD card interface
		sd_cs : out std_logic;
		sd_miso : in std_logic;
		sd_mosi : out std_logic;
		sd_clk : out std_logic;
		
		-- Power and LEDs
		power_button : in std_logic;
		power_hold : out std_logic := '1';
		leds : out std_logic_vector(3 downto 0);
		
		-- Any remaining IOs yet to be assigned
		misc_ios_1 : in std_logic_vector(5 downto 0);
		misc_ios_21 : in std_logic_vector(13 downto 0);
		misc_ios_22 : in std_logic_vector(8 downto 0);
		misc_ios_3 : in std_logic_vector(1 downto 0)
	);
end entity;

architecture RTL of C3BoardToplevel is
-- Assigns pin location to ports on an entity.
-- Declare the attribute or import its declaration from 
-- altera.altera_syn_attributes
attribute chip_pin : string;

-- Board features

attribute chip_pin of clk_50 : signal is "152";
attribute chip_pin of reset_n : signal is "181";
attribute chip_pin of led_out : signal is "233";

-- SDRAM (2 distinct 8-bit wide chips)

attribute chip_pin of sd1_addr : signal is "83,69,82,81,80,78,99,110,63,64,65,68";
attribute chip_pin of sd1_data : signal is "109,103,111,93,100,106,107,108";
attribute chip_pin of sd1_ba : signal is "70,71";
attribute chip_pin of sdram1_clk : signal is "117";
attribute chip_pin of sd1_cke : signal is "84";
attribute chip_pin of sd1_dqm : signal is "87";
attribute chip_pin of sd1_cs : signal is "72";
attribute chip_pin of sd1_we : signal is "88";
attribute chip_pin of sd1_cas : signal is "76";
attribute chip_pin of sd1_ras : signal is "73";

attribute chip_pin of sd2_addr : signal is "142,114,144,139,137,134,148,161,120,119,118,113";
attribute chip_pin of sd2_data : signal is "166,164,162,160,146,147,159,168";
attribute chip_pin of sd2_ba : signal is "126,127";
attribute chip_pin of sdram2_clk : signal is "186";
attribute chip_pin of sd2_cke : signal is "143";
attribute chip_pin of sd2_dqm : signal is "145";
attribute chip_pin of sd2_cs : signal is "128";
attribute chip_pin of sd2_we : signal is "133";
attribute chip_pin of sd2_cas : signal is "132";
attribute chip_pin of sd2_ras : signal is "131";

-- Video output via custom board

attribute chip_pin of vga_red : signal is "13, 9, 5, 240, 238, 236";
attribute chip_pin of vga_green : signal is "49, 45, 43, 39, 37, 18";
attribute chip_pin of vga_blue : signal is "52, 50, 46, 44, 41, 38";

attribute chip_pin of vga_hsync : signal is "51";
attribute chip_pin of vga_vsync : signal is "55";

-- Audio output via custom board

attribute chip_pin of aud_l : signal is "6";
attribute chip_pin of aud_r : signal is "22";

-- PS/2 sockets on custom board

attribute chip_pin of ps2k_clk : signal is "235";
attribute chip_pin of ps2k_dat : signal is "237";
attribute chip_pin of ps2m_clk : signal is "239";
attribute chip_pin of ps2m_dat : signal is "4";

-- RS232
attribute chip_pin of rs232_rxd : signal is "98";
attribute chip_pin of rs232_txd : signal is "112";

-- SD card interface
attribute chip_pin of sd_cs : signal is "185";
attribute chip_pin of sd_miso : signal is "196";
attribute chip_pin of sd_mosi : signal is "188";
attribute chip_pin of sd_clk : signal is "194";


-- Power and LEDs
attribute chip_pin of power_hold : signal is "171";
attribute chip_pin of power_button : signal is "94";

attribute chip_pin of leds : signal is "173, 169, 167, 135";

attribute chip_pin of btn1 : signal is "226";
attribute chip_pin of btn2 : signal is "231";

-- Free pins, not yet assigned

attribute chip_pin of misc_ios_1 : signal is "12,14,56,234,21,57";

attribute chip_pin of misc_ios_21 : signal is "184,187,189,195,197,201,203,214,217,219,221,223,232";
attribute chip_pin of misc_ios_22 : signal is "176,183,200,202,207,216,218,224,230";
attribute chip_pin of misc_ios_3 : signal is "95,177";

-- Signals internal to the project

signal sysclk : std_logic;
signal memclk : std_logic;
signal reset : std_logic;  -- active low
signal pll1_locked : std_logic;
signal pll2_locked : std_logic;

signal debugvalue : std_logic_vector(15 downto 0);

signal btn1_d : std_logic;
signal btn2_d : std_logic;

signal currentX : unsigned(11 downto 0);
signal currentY : unsigned(11 downto 0);
signal end_of_pixel : std_logic;
signal end_of_line : std_logic;
signal end_of_frame : std_logic;

-- SDRAM - merged signals to make the two chips appear as a single 16-bit wide entity.
signal sdr_addr : std_logic_vector(11 downto 0);
signal sdr_dqm : std_logic_vector(1 downto 0);
signal sdr_we : std_logic;
signal sdr_cas : std_logic;
signal sdr_ras : std_logic;
signal sdr_cs : std_logic;
signal sdr_ba : std_logic_vector(1 downto 0);
-- signal sdr_clk : std_logic;
signal sdr_cke : std_logic;

signal ps2m_clk_in : std_logic;
signal ps2m_clk_out : std_logic;
signal ps2m_dat_in : std_logic;
signal ps2m_dat_out : std_logic;

signal ps2k_clk_in : std_logic;
signal ps2k_clk_out : std_logic;
signal ps2k_dat_in : std_logic;
signal ps2k_dat_out : std_logic;

signal power_led : unsigned(5 downto 0);
signal disk_led : unsigned(5 downto 0);
signal net_led : unsigned(5 downto 0);
signal odd_led : unsigned(5 downto 0);

signal vga_r : unsigned(7 downto 0);
signal vga_g : unsigned(7 downto 0);
signal vga_b : unsigned(7 downto 0);
signal vga_window : std_logic;

signal audio_l : signed(15 downto 0);
signal audio_r : signed(15 downto 0);

-- Video dither
COMPONENT video_vga_dither
	generic (
		outbits : integer :=4
		);
	port (
		clk : in std_logic;
		hsync : in std_logic;
		vsync : in std_logic;
		vid_ena : in std_logic;
		iRed : in unsigned(7 downto 0);
		iGreen : in unsigned(7 downto 0);
		iBlue : in unsigned(7 downto 0);
		oRed : out unsigned(outbits-1 downto 0);
		oGreen : out unsigned(outbits-1 downto 0);
		oBlue : out unsigned(outbits-1 downto 0)
	);
end component;


-- Sigma Delta audio
COMPONENT hybrid_pwm_sd
	PORT
	(
		clk		:	 IN STD_LOGIC;
		n_reset		:	 IN STD_LOGIC;
		din		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		dout		:	 OUT STD_LOGIC
	);
END COMPONENT;

begin

	power_led(5 downto 0)<="001111";
	disk_led(5 downto 2)<=unsigned(debugvalue(11 downto 8));
	net_led(5 downto 2)<=unsigned(debugvalue(7 downto 4));
	odd_led(5 downto 2)<=unsigned(debugvalue(3 downto 0));

	myreset : entity work.poweronreset
	port map(
		clk => sysclk,
		reset_button => power_button, --  and pll1_locked and pll2_locked,
		reset_out => reset,
		power_button => '1',
		power_hold => power_hold		
	);

	myleds : entity work.statusleds_pwm
	port map(
		clk => sysclk,
		power_led => power_led,
		disk_led => disk_led,
		net_led => net_led,
		odd_led => odd_led,
		leds_out => leds
	);
	
	ps2m_dat_in<=ps2m_dat;
	ps2m_dat <= '0' when ps2m_dat_out='0' else 'Z';
	ps2m_clk_in<=ps2m_clk;
	ps2m_clk <= '0' when ps2m_clk_out='0' else 'Z';

	ps2k_dat_in<=ps2k_dat;
	ps2k_dat <= '0' when ps2k_dat_out='0' else 'Z';
	ps2k_clk_in<=ps2k_clk;
	ps2k_clk <= '0' when ps2k_clk_out='0' else 'Z';

	sd1_addr <= sdr_addr;
	sd1_dqm <= sdr_dqm(0);
--	sd1_clk <= sdr_clk;
	sd1_we <= sdr_we;
	sd1_cas <= sdr_cas;
	sd1_ras <= sdr_ras;
	sd1_cs <= sdr_cs;
	sd1_ba <= sdr_ba;
	sd1_cke <= sdr_cke;

	sd2_addr <= sdr_addr;
	sd2_dqm <= sdr_dqm(1);
--	sd2_clk <= sdr_clk;
	sd2_we <= sdr_we;
	sd2_cas <= sdr_cas;
	sd2_ras <= sdr_ras;
	sd2_cs <= sdr_cs;
	sd2_ba <= sdr_ba;
	sd2_cke <= sdr_cke;
		
	mypll : entity work.PLL
		port map (
			inclk0 => clk_50,
			c2 => sysclk,
			c1 => memclk,
			c0 => sdram1_clk
		);
		
	mypll2 : entity work.PLL2
		port map (
			inclk0 => clk_50,
			c0 => sdram2_clk
		);

myvirtualtoplevel : entity work.Virtual_Toplevel
	port map(
		reset => reset,
		CLK => sysclk,
		SDR_CLK => memclk,

		 -- SDRAM DE1 ports
		DRAM_CKE => sdr_cke,
		DRAM_CS_N => sdr_cs,
		DRAM_RAS_N => sdr_ras,
		DRAM_CAS_N => sdr_cas,
		DRAM_WE_N => sdr_we,
		DRAM_UDQM => sdr_dqm(1),
		DRAM_LDQM => sdr_dqm(0),
		DRAM_BA_1 => sdr_ba(1),
		DRAM_BA_0 => sdr_ba(0),
		DRAM_ADDR => sdr_addr(11 downto 0),
		DRAM_DQ(15 downto 8) => sd2_data,
		DRAM_DQ(7 downto 0) => sd1_data,

		 -- PS/2 keyboard ports
		ps2k_clk_out => ps2k_clk_out,
		ps2k_dat_out => ps2k_dat_out,
		ps2k_clk_in => ps2k_clk_in,
		ps2k_dat_in => ps2k_dat_in,
		 
		--    -- Joystick ports (Port_A, Port_B)
		joya => "11111111", -- joya,
		joyb => "11111111", -- joyb,

		-- UART
		RS232_RXD => rs232_rxd,
		RS232_TXD => rs232_txd,
			
		-- SD Card interface
		spi_cs => sd_cs,
		spi_miso => sd_miso,
		spi_mosi => sd_mosi,
		spi_clk => sd_clk,

		-- Audio
		signed(DAC_LDATA) => audio_l,
		signed(DAC_RDATA) => audio_r,

		-- Video
		unsigned(VGA_R) => vga_r,
		unsigned(VGA_G) => vga_g,
		unsigned(VGA_B) => vga_b,

		VGA_HS => vga_hsync,
		VGA_VS => vga_vsync
);

vga_window<='1';

	mydither : component video_vga_dither
		generic map(
			outbits => 6
	)
		port map(
			clk=>memclk,
			hsync=>vga_hsync,
			vsync=>vga_vsync,
			vid_ena=>vga_window,
			iRed => vga_r,
			iGreen => vga_g,
			iBlue => vga_b,
			oRed => vga_red,
			oGreen => vga_green,
			oBlue => vga_blue
		);

		-- Do we have audio?  If so, instantiate a two DAC channels.
leftsd: component hybrid_pwm_sd
	port map
	(
		clk => memclk,
		n_reset => reset,
		din(15) => not audio_l(15),
		din(14 downto 0) => std_logic_vector(audio_l(14 downto 0)),
		dout => aud_l
	);
	
rightsd: component hybrid_pwm_sd
	port map
	(
		clk => memclk,
		n_reset => reset,
		din(15) => not audio_r(15),
		din(14 downto 0) => std_logic_vector(audio_r(14 downto 0)),
		dout => aud_r
	);

end RTL;

