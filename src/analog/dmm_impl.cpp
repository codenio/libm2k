/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of libm2k
 * (see http://www.github.com/analogdevicesinc/libm2k).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "dmm_impl.hpp"
#include "utils/devicegeneric.hpp"
#include "utils/devicein.hpp"
#include <libm2k/m2kexceptions.hpp>
#include "utils/channel.hpp"
#include <iio.h>
#include <iostream>

using namespace libm2k::analog;
using namespace libm2k::utils;


DMMImpl::DMMImpl(struct iio_context *ctx, std::string dev, bool sync)
{
	UNUSED(sync);
	m_device_in_list.push_back(new DeviceIn(ctx, dev));
	m_dev_name = dev;
	bool output = false;
	for (unsigned int i = 0; i < getDevice(0)->getNbChannels(output); i++) {
		if (getDevice(0)->isValidDmmChannel(i)) {
			m_channel_id_list.insert(std::pair<std::string, unsigned int>
						 (getDevice(0)->getChannel(i, output)->getId(), i));
		}
	}
}

DMMImpl::~DMMImpl()
{
	for (auto d : m_device_in_list) {
		delete d;
	}
	m_device_in_list.clear();
}

void DMMImpl::reset()
{

}

DeviceIn* DMMImpl::getDevice(unsigned int index)
{
	if (index >= m_device_in_list.size()) {
		THROW_M2K_EXCEPTION("No such DMM device.", libm2k::EXC_OUT_OF_RANGE);
	}
	return m_device_in_list.at(index);
}

DMM_READING DMMImpl::readChannel(unsigned int index)
{
	std::string chn_name = "";
	for (auto pair : m_channel_id_list) {
		if (pair.second == index) {
			chn_name = pair.first;
			break;
		}
	}
	return readChannel(chn_name);
}

std::vector<std::string> DMMImpl::getAllChannels()
{
	std::vector<std::string> ids = {};
	for (auto pair : m_channel_id_list) {
		ids.push_back(pair.first);
	}
	return ids;
}

DMM_READING DMMImpl::readChannel(std::string chn_name)
{
	DMM_READING result;
	double value = 0;
	std::string key = "";
	std::string key_symbol = "";
	std::string id = getDevice(0)->getChannel(m_channel_id_list.at(chn_name), false)->getId();
	std::string name = getDevice(0)->getChannel(m_channel_id_list.at(chn_name), false)->getName();
	unsigned int index = m_channel_id_list.at(chn_name);
	auto channel = getDevice(0)->getChannel(index, false);
	if (channel->hasAttribute("raw")) {
		value = channel->getDoubleValue("raw");
	} else if (channel->hasAttribute("processed")) {
		value = channel->getDoubleValue("processed");
	} else if (channel->hasAttribute("input")) {
		value = channel->getDoubleValue("input");
	} else {
		THROW_M2K_EXCEPTION("DMM: Cannot read channel " + getName() + " : " + chn_name, libm2k::EXC_OUT_OF_RANGE);
	}

	if (channel->hasAttribute("offset")) {
		value += channel->getDoubleValue("offset");
	}

	if (channel->hasAttribute("scale")) {
		value *= channel->getDoubleValue("scale");
	}


	if (chn_name.find("voltage") != std::string::npos) {
		key = "Volt";
		key_symbol = "V";
		value = value / 1000;
	} else if (chn_name.find("temp") != std::string::npos) {
		key = "Degree Celsius";
		key_symbol = "°C";
		value = value / 1000;
	} else if (chn_name.find("current") != std::string::npos) {
		key = "Ampere";
		key_symbol = "A";
		value = value / 1000;
	} else if (chn_name.find("accel") != std::string::npos) {
		key =  "Metre per second squared";
		key_symbol = "m/s²";
	} else if (chn_name.find("anglvel") != std::string::npos) {
		key = "Radian per second";
		key_symbol = "rad/s";
	} else if (chn_name.find("pressure") != std::string::npos) {
		key = "Pascal";
		key_symbol = "Pa";
		value = value * 1000;
	} else if (chn_name.find("magn") != std::string::npos) {
		key = "Gauss";
		key_symbol = "G";
	} else {
		key = "";
		key_symbol = "";
	}

	result.id = id;
	result.name = name;
	result.unit_name = key;
	result.unit_symbol = key_symbol;
	result.value = value;
	return result;
}

std::vector<DMM_READING> DMMImpl::readAll()
{
	std::vector<DMM_READING> result = {};
	for (auto pair : m_channel_id_list) {
		DMM_READING res = readChannel(pair.first);
		result.push_back(res);
	}
	return result;
}

string DMMImpl::getName()
{
	return getDevice(0)->getName();
}

