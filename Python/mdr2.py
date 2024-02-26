""" MDR-2 monochromator class unit"""

__version__ = '26.02.2024'
__author__ = 'Serhiy Kobyakov'

import time
import math
import configparser
from arduino_device import ArduinoDevice


class MDR2(ArduinoDevice):
    """ MDR-2 monochromator class """
    # define the device name:
    # this is the string with which the device responds to b'?' query
    _device_name = "MDR-2"

    # other device-specific variables go here:

    def __init__(self, comport):
        """Initialization of MDR-2 monochromator"""

        # init the list for motor pos -> wavelength coefficients
        self.__G = []
        self.__G.append([0, 0])

        # repeat assigning class variables,
        # so they are visible in self.__dict__:
        self._device_name = self._device_name

        # read the device parameters from INI file
        # all except COMPORTSPEED, READTIMEOUT, WRITETIMEOUT,
        # LONGREADTIMEOUT and SHORTESTTIMEBETWEENREADS:
        # self.[some parameter] =
        #    config[self._device_name]['some parameter']
        inifname = self._device_name + '.INI'
        with open(inifname, "r") as f:
            config = configparser.ConfigParser()
            config.read_file(f)
            self.__MaxPos = \
                config.getint(self._device_name, 'MaxPos')
            self.__rDisp = \
                config[self._device_name]['rDisp']
            self.__Grating = \
                config.getint(self._device_name, 'Grating')
            for g in range(1, 4):
                self.__G.append([config.getfloat(self._device_name,
                                                 f"G{g}A0"),
                                config.getfloat(self._device_name,
                                                f"G{g}A1")])

        # start serial communication with the device
        # this is the place for the line!
        super().__init__(comport)

    def __del__(self):
        # do some default device-specific finalization actions here:

        # park grating
        self.send_and_get_late_answer('z')

        # save to the ini file the actual grating number
        # if you use other gratings than the first one

        # this is the place for the line!
        super().__del__()

    def do_init(self):
        """command the monochromator to initialize itself"""
        self.send_and_get_late_answer('i')

    def pos_in_range(self, pos) -> bool:
        """check if given stepper motor position
        is in the allowed range"""
        if 0 <= int(pos) <= self.__MaxPos:
            return True
        else:
            return False

    def get_grating(self) -> int:
        return self.__Grating

    def get_min_pos_nm(self) -> float:
        """returns the minimal wavelength position
        using actual grating"""
        return math.ceil(self.__G[self.__Grating][0] +
                         500 * self.__G[self.__Grating][1]) + 1.

    def get_max_pos_nm(self) -> float:
        """returns the maximum wavelength position
        using actual grating"""
        return math.floor(self.__G[self.__Grating][0] +
                          (self.__MaxPos - 10) *
                          self.__G[self.__Grating][1]) - 1.

    def get_pos(self) -> int:
        """get actual stepper motor position of MDR-2"""
        return int(self.send_and_get_answer('p'))

    def get_pos_nm(self) -> float:
        """get actual MDR-2 monochromator position in nanometers"""
        pos = int(self.send_and_get_answer('p'))
        return round(self.__G[self.__Grating][0] +
                     pos * self.__G[self.__Grating][1], 3)

    def go_to_pos(self, pos):
        """go to the stepper motor MDR-2 position"""
        if self.pos_in_range(pos):
            self.send_and_get_late_answer('g' + str(pos))
            time.sleep(0.25)
        else:
            print(f"pos {pos} is out of the \
range 0..{self.__MaxPos}!!!")

    def go_to_pos_nm(self, pos_nm):
        """set the MDR-2 monochromator to wavelength: pos_nm"""
        pos = round((pos_nm - self.__G[self.__Grating][0]) /
                    self.__G[self.__Grating][1])
        self.go_to_pos(pos)
