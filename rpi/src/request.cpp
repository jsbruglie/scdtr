/**
 * @file    rpi/src/request.cpp
 * @brief   Server request implementation
 * 
 * Implements server request parsing and response functions
 * 
 * @author João Borrego
 */

#include "request.hpp"

void streamUpdate(
    System::ptr system,
    std::vector< unsigned long > & timestamps,
    const std::vector< bool> & flags,
    std::string & response)
{
    int nodes = system->getNodes();
    float var;
    Entry *entry;
    response = "";

    for (int i = 0; i < nodes; i++)
    {
        entry = system->getLatestEntry((size_t) i);
        if (entry != NULL){
            if (entry->timestamp > timestamps[i]){
                if (flags[STREAM_FLAGS * i]){
                    var = entry->lux;
                    if (var != -1){
                        response += "c " + std::string(1, LUX) + " "
                            + std::to_string(i) + " " + std::to_string(var)
                            + " " + std::to_string(entry->timestamp) + " ";
                    }
                }
                if (flags[STREAM_FLAGS * i + 1]){
                    var = entry->duty_cycle;
                    if (var != -1){
                        response += "c " + std::string(1, DUTY_CYCLE) + " "
                            + std::to_string(i) + " " + std::to_string(var)
                            + " " + std::to_string(entry->timestamp) + " ";
                    }
                }
                timestamps[i] = entry->timestamp;
            }
        }
    }
    //debugPrintTrace(response);
}

void parseRequest(
    System::ptr system,
    std::vector< unsigned long > & timestamps,
    std::vector< bool> & flags,
    const std::string & request,
    std::string & response)
{
    std::istringstream iss(request);
    std::string type, cmd, arg;
    int id = -1;
    bool total = false;

    unsigned long now = system->millis();
    long minute_ago = now - (60 * 1000);
    if (minute_ago < 0) minute_ago = 0;

    if (!system)
    {
        response = INVALID;
        return;
    }

    try
    {
        iss >> type >> cmd >> arg;
    }
    catch (std::exception & e)
    {
        errPrintTrace(e.what());
        return;
    }

    if (type.size() == 1)
    {
        if (type == RESET)
        {
            system->startWriteSerial(RESET);
            system->reset();
            for (int i = 0; i < system->getNodes(); i++){
                timestamps.at(i) = 0;
                for (int j = 0; j < STREAM_FLAGS; j++){
                    flags.at(i * STREAM_FLAGS + j) = false; 
                }
            }
            response = ACK;
        }
        else if (type == DISTRIBUTED_ON)
        {
            system->startWriteSerial(DISTRIBUTED_ON);
        }
        else if (type == DISTRIBUTED_OFF)
        {
            system->startWriteSerial(DISTRIBUTED_OFF);
        }
        else if (type == SAVE)
        {
            system->saveEntries();
        }
        else
        {
            if (cmd.size() == 1)
            {
                if (type == GET)
                {
                    char param = cmd[0];

                    if (arg.size() == 1 && arg[0] == TOTAL)
                    {
                        if (param == POWER || param == ENERGY ||
                            param == COMFORT_ERR || param == COMFORT_VAR)
                        {
                            total = true;
                        }
                        else
                        {
                            response = INVALID;
                            return;
                        }
                    }
                    else
                    {
                        try
                        {
                            id = std::stoi(arg);
                            if (id < 0 || id >= system->getNodes()) throw std::exception();
                        }
                        catch (std::exception e)
                        {
                            response = INVALID;
                            return;
                        }
                    }

                    // Prevent cross initialisation errors in switch
                    float value_f = -1;
                    int value_i = -1;
                    bool value_b = 0;
                    long value_l = -1;

                    std::string id_str(std::to_string(id));

                    switch (param)
                    {
                        case LUX:
                            value_f = system->getLux(id);
                            response = std::string(1,LUX) + " " + id_str
                                + " " + std::to_string(value_f);
                            break;
                        case DUTY_CYCLE:
                            value_f = system->getDutyCycle(id);
                            response = std::string(1, DUTY_CYCLE) + " " + id_str
                                + " " + std::to_string(value_f);
                            break;
                        case OCCUPANCY:
                            value_b = system->getOccupancy(id);
                            response = std::string(1, OCCUPANCY) + " " + id_str
                                + " " + std::to_string(value_b);
                            break;
                        case LUX_LOWER:
                            value_f = system->getLuxLowerBound(id);
                            response = std::string(1, LUX_LOWER) + " " + id_str
                                + " " + std::to_string(value_f);
                            break;
                        case LUX_EXTERNAL:
                            value_f = system->getLuxExternal(id);
                            response = std::string(1, LUX_EXTERNAL) + " " + id_str
                                + " " + std::to_string(value_f);
                            break;
                        case LUX_REF:
                            value_f = system->getLuxReference(id);
                            response =  std::string(1, LUX_REF) + " " + id_str
                                + " " + std::to_string(value_f);
                            break;
                        case POWER:
                            value_f = system->getPower(id, total);
                            response =  std::string(1, POWER) + " " +
                                ((id == -1 && total)? std::string(1, TOTAL) : id_str)
                                + " " + std::to_string(value_f);
                            break;
                        case ENERGY:
                            value_f = system->getEnergy(id, total);
                            response =  std::string(1, ENERGY) + " " +
                                ((id == -1 && total)? std::string(1, TOTAL) : id_str)
                                + " " + std::to_string(value_f);
                            break;
                        case COMFORT_ERR:
                            value_f = system->getComfortError(id, total);
                            response =  std::string(1, COMFORT_ERR) + " " +
                                ((id == -1 && total)? std::string(1, TOTAL) : id_str)
                                + " " + std::to_string(value_f);
                            break;
                        case COMFORT_VAR:
                            value_f = system->getComfortVariance(id, total);
                            response =  std::string(1, COMFORT_VAR) + " " +
                                ((id == -1 && total)? std::string(1, TOTAL) : id_str)
                                + " " + std::to_string(value_f);
                            break;
                        case TIMESTAMP:
                            value_l  = system->getTimestamp(id);
                            response =  std::string(1, TIMESTAMP) + " " + id_str
                                + " " + std::to_string((float) value_l / 1000.0);
                            break;
                        default:
                            response = INVALID;
                    }
                }
                else if (type == SET)
                {
                    try
                    {
                        int id = std::stoi(cmd);
                        if (id < 0 || id >= system->getNodes()) throw std::exception();
                        bool val = std::stoi(arg);
                        std::string serial_msg("");
                        system->startWriteSerial(request);
                        response = ACK;
                    }
                    catch (std::exception e)
                    {
                        response = INVALID;
                    }
                }
                else if (type == START_STREAM || type == STOP_STREAM || type == LAST_MINUTE)
                {
                    if (cmd.size() == 1)
                    {
                        char var = cmd[0];
                        try
                        {
                            id = std::stoi(arg);
                            if (id < 0 || id >= system->getNodes()) throw std::exception();
                        }
                        catch (std::exception e)
                        {
                            response = INVALID;
                            return;
                        }

                        if (type == LAST_MINUTE)
                        {
                            system->getValuesInPeriod(id, minute_ago, now, var, response);
                        }
                        else
                        {
                            switch(var)
                            {
                                case LUX:
                                    flags[id * STREAM_FLAGS] = (type == START_STREAM);
                                    break;
                                case DUTY_CYCLE:
                                    flags[id * STREAM_FLAGS + 1] = (type == START_STREAM);
                                    break;
                                default:
                                    response = INVALID;
                            }
                        }
                    }
                }
                else
                {
                    response = INVALID;
                }
            }
            else
            {
                response = INVALID;
            }
        }
    }
    else
    {
        response = INVALID;
    }
}
