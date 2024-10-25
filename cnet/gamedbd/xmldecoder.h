#ifndef __XMLDECODER_H
#define __XMLDECODER_H

#include <cstdio>
#include <cstring>

#include "octets.h"
#include "conv_charset.h"

#include "tinyxml2.h"
#include "groledata"


namespace GNET
{

Octets XMLGetOctets(const char* content, int len)
{
    if ((content == NULL) || (len <= 0) || (len % 2 != 0)) return Octets();

    unsigned char c = 0;
    Octets value(sizeof(unsigned char) * (len / 2));

    for (int i = 0; i < len; i += 2)
    {
        sscanf(content + i, "%02hhx", &c);
        value.insert(value.end(), &c, sizeof(c));
    }

    return value;
}


class XmlDecoder
{
public:
    static bool Decode(const char* xmlfile, GRoleData& x)
    {
        tinyxml2::XMLDocument doc;
        if ((xmlfile == NULL) || (doc.LoadFile(xmlfile) != 0))
        {
            printf("load xmlfile error.\n");
            return false;
        }

        tinyxml2::XMLElement* root = doc.RootElement();
        if (root == NULL)
        {
            printf("read root element error.\n");
            return false;
        }

        const tinyxml2::XMLElement* pIter = root->FirstChildElement();
        for (; pIter != NULL; pIter = pIter->NextSiblingElement())
        {
            const char* child_name = pIter->Name();
            if (child_name == NULL) return false;

            if (strcmp(child_name, "variable") == 0)
            {
                const char* name = pIter->Attribute("name");
                if (name == NULL) return false;
                const char* content = pIter->GetText();
                if (content == NULL) content = "";

                printf("error member (%s, %s) in GRoleData.\n", name, content);
                return false;
            }
            else if (strcmp(child_name, "base") == 0)
            {
                if (!Decode(pIter, x.base)) return false;
            }
            else if (strcmp(child_name, "status") == 0)
            {
                if (!Decode(pIter, x.status)) return false;
            }
            else if (strcmp(child_name, "pocket") == 0)
            {
                if (!Decode(pIter, x.pocket)) return false;
            }
            else if (strcmp(child_name, "equipment") == 0)
            {
                if (!Decode(pIter, x.equipment)) return false;
            }
            else if (strcmp(child_name, "storehouse") == 0)
            {
                if (!Decode(pIter, x.storehouse)) return false;
            }
            else if (strcmp(child_name, "task") == 0)
            {
                if (!Decode(pIter, x.task)) return false;
            }
            else
            {
                printf("error member (%s) in GRoleData.\n", child_name);
                return false;
            }
        }

        return true;
	}

    static bool Decode(const tinyxml2::XMLElement* pChild, GRoleBase& x)
    {
        if (pChild == NULL) return false;

        const tinyxml2::XMLElement* pIter = pChild->FirstChildElement();
        for (; pIter != NULL; pIter = pIter->NextSiblingElement())
        {
            const char* child_name = pIter->Name();
            if (child_name == NULL) return false;

            if (strcmp(child_name, "variable") == 0)
            {
                const char* name = pIter->Attribute("name");
                if (name == NULL) return false;
                const char* content = pIter->GetText();
                if (content == NULL) content = "";

                if (strcmp(name, "version") == 0)
                    x.version = (char)strtol(content, 0, 10);
                else if (strcmp(name, "id") == 0)
                    x.id = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "name") == 0)
                {
                    Octets str_out;
                    Octets str_in(content, strlen(content));
                    CharsetConverter::conv_charset_t2u(str_in, str_out);
                    x.name = str_out;
                }
                else if (strcmp(name, "race") == 0)
                    x.race = (int)strtol(content, 0, 10);
                else if (strcmp(name, "cls") == 0)
                    x.cls = (int)strtol(content, 0, 10);
                else if (strcmp(name, "gender") == 0)
                    x.gender = (unsigned char)strtol(content, 0, 10);
                else if (strcmp(name, "custom_data") == 0)
                    x.custom_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "config_data") == 0)
                    x.config_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "custom_stamp") == 0)
                    x.custom_stamp = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "status") == 0)
                    x.status = (unsigned char)strtol(content, 0, 10);
                else if (strcmp(name, "delete_time") == 0)
                    x.delete_time = (int)strtol(content, 0, 10);
                else if (strcmp(name, "create_time") == 0)
                    x.create_time = (int)strtol(content, 0, 10);
                else if (strcmp(name, "lastlogin_time") == 0)
                    x.lastlogin_time = (int)strtol(content, 0, 10);
                else if (strcmp(name, "help_states") == 0)
                    x.help_states = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "spouse") == 0)
                    x.spouse = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "userid") == 0)
                    x.userid = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "cross_data") == 0)
                    x.cross_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "reserved2") == 0)
                    x.reserved2 = (unsigned char)strtol(content, 0, 10);
                else if (strcmp(name, "reserved3") == 0)
                    x.reserved3 = (unsigned char)strtol(content, 0, 10);
                else if (strcmp(name, "reserved4") == 0)
                    x.reserved4 = (unsigned char)strtol(content, 0, 10);
                else
                {
                    printf("error member (%s, %s) in GRoleBase.\n", name, content);
                    return false;
                }
            }
            else if (strcmp(child_name, "forbid") == 0)
            {
                if (!Decode(pIter, x.forbid)) return false;
            }
            else
            {
                printf("error member (%s) in GRoleBase.\n", child_name);
                return false;
            }
        }

        return true;
    }

    static bool Decode(const tinyxml2::XMLElement* pChild, GRoleStatus& x)
    {
        if (pChild == NULL) return false;

        const tinyxml2::XMLElement* pIter = pChild->FirstChildElement();
        for (; pIter != NULL; pIter = pIter->NextSiblingElement())
        {
            const char* child_name = pIter->Name();
            if (child_name == NULL) return false;

            if (strcmp(child_name, "variable") == 0)
            {
                const char* name = pIter->Attribute("name");
                if (name == NULL) return false;
                const char* content = pIter->GetText();
                if (content == NULL) content = "";

                if (strcmp(name, "version") == 0)
                    x.version = (char)strtol(content, 0, 10);
                else if (strcmp(name, "level") == 0)
                    x.level = (int)strtol(content, 0, 10);
                else if (strcmp(name, "level2") == 0)
                    x.level2 = (int)strtol(content, 0, 10);
                else if (strcmp(name, "exp") == 0)
                    x.exp = (int)strtol(content, 0, 10);
                else if (strcmp(name, "sp") == 0)
                    x.sp = (int)strtol(content, 0, 10);
                else if (strcmp(name, "pp") == 0)
                    x.pp = (int)strtol(content, 0, 10);
                else if (strcmp(name, "hp") == 0)
                    x.hp = (int)strtol(content, 0, 10);
                else if (strcmp(name, "mp") == 0)
                    x.mp = (int)strtol(content, 0, 10);
                else if (strcmp(name, "posx") == 0)
                    x.posx = (float)strtof(content, 0);
                else if (strcmp(name, "posy") == 0)
                    x.posy = (float)strtof(content, 0);
                else if (strcmp(name, "posz") == 0)
                    x.posz = (float)strtof(content, 0);
                else if (strcmp(name, "worldtag") == 0)
                    x.worldtag = (int)strtol(content, 0, 10);
                else if (strcmp(name, "invader_state") == 0)
                    x.invader_state = (int)strtol(content, 0, 10);
                else if (strcmp(name, "invader_time") == 0)
                    x.invader_time = (int)strtol(content, 0, 10);
                else if (strcmp(name, "pariah_time") == 0)
                    x.pariah_time = (int)strtol(content, 0, 10);
                else if (strcmp(name, "reputation") == 0)
                    x.reputation = (int)strtol(content, 0, 10);
                else if (strcmp(name, "custom_status") == 0)
                    x.custom_status = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "filter_data") == 0)
                    x.filter_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "charactermode") == 0)
                    x.charactermode = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "instancekeylist") == 0)
                    x.instancekeylist = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "dbltime_expire") == 0)
                    x.dbltime_expire = (int)strtol(content, 0, 10);
                else if (strcmp(name, "dbltime_mode") == 0)
                    x.dbltime_mode = (int)strtol(content, 0, 10);
                else if (strcmp(name, "dbltime_begin") == 0)
                    x.dbltime_begin = (int)strtol(content, 0, 10);
                else if (strcmp(name, "dbltime_used") == 0)
                    x.dbltime_used = (int)strtol(content, 0, 10);
                else if (strcmp(name, "dbltime_max") == 0)
                    x.dbltime_max = (int)strtol(content, 0, 10);
                else if (strcmp(name, "time_used") == 0)
                    x.time_used = (int)strtol(content, 0, 10);
                else if (strcmp(name, "dbltime_data") == 0)
                    x.dbltime_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "storesize") == 0)
                    x.storesize = (unsigned short)strtol(content, 0, 10);
                else if (strcmp(name, "petcorral") == 0)
                    x.petcorral = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "property") == 0)
                    x.property = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "var_data") == 0)
                    x.var_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "skills") == 0)
                    x.skills = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "storehousepasswd") == 0)
                    x.storehousepasswd = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "waypointlist") == 0)
                    x.waypointlist = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "coolingtime") == 0)
                    x.coolingtime = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "npc_relation") == 0)
                    x.npc_relation = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "multi_exp_ctrl") == 0)
                    x.multi_exp_ctrl = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "storage_task") == 0)
                    x.storage_task = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "faction_contrib") == 0)
                    x.faction_contrib = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "force_data") == 0)
                    x.force_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "online_award") == 0)
                    x.online_award = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "profit_time_data") == 0)
                    x.profit_time_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "country_data") == 0)
                    x.country_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "king_data") == 0)
                    x.king_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "meridian_data") == 0)
                    x.meridian_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "extraprop") == 0)
                    x.extraprop = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "title_data") == 0)
                    x.title_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "reincarnation_data") == 0)
                    x.reincarnation_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "realm_data") == 0)
                    x.realm_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "reserved2") == 0)
                    x.reserved2 = (char)strtol(content, 0, 10);
                else if (strcmp(name, "reserved3") == 0)
                    x.reserved3 = (char)strtol(content, 0, 10);
                else
                {
                    printf("error member (%s, %s) in GRoleStatus.\n", name, content);
                    return false;
                }
            }
            else
            {
                printf("error member (%s) in GRoleStatus.\n", child_name);
                return false;
            }
        }

        return true;
    }

    static bool Decode(const tinyxml2::XMLElement* pChild, GRolePocket& x)
    {
        if (pChild == NULL) return false;

        const tinyxml2::XMLElement* pIter = pChild->FirstChildElement();
        for (; pIter != NULL; pIter = pIter->NextSiblingElement())
        {
            const char* child_name = pIter->Name();
            if (child_name == NULL) return false;

            if (strcmp(child_name, "variable") == 0)
            {
                const char* name = pIter->Attribute("name");
                if (name == NULL) return false;
                const char* content = pIter->GetText();
                if (content == NULL) content = "";

                if (strcmp(name, "capacity") == 0)
                    x.capacity = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "timestamp") == 0)
                    x.timestamp = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "money") == 0)
                    x.money = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "reserved1") == 0)
                    x.reserved1 = (int)strtol(content, 0, 10);
                else if (strcmp(name, "reserved2") == 0)
                    x.reserved2 = (int)strtol(content, 0, 10);
                else
                {
                    printf("error member (%s, %s) in GRolePocket.\n", name, content);
                    return false;
                }
            }
            else if (strcmp(child_name, "items") == 0)
            {
                if (!Decode(pIter, x.items)) return false;
            }
            else
            {
                printf("error member (%s) in GRolePocket.\n", child_name);
                return false;
            }
        }

        return true;
    }

    static bool Decode(const tinyxml2::XMLElement* pChild, GRoleEquipment& x)
    {
        if (pChild == NULL) return false;

        const tinyxml2::XMLElement* pIter = pChild->FirstChildElement();
        for (; pIter != NULL; pIter = pIter->NextSiblingElement())
        {
            const char* child_name = pIter->Name();
            if (child_name == NULL) return false;

            if (strcmp(child_name, "variable") == 0)
            {
                const char* name = pIter->Attribute("name");
                if (name == NULL) return false;
                const char* content = pIter->GetText();
                if (content == NULL) content = "";

                printf("error member (%s, %s) in GRoleEquipment.\n", name, content);
                return false;
            }
            else if (strcmp(child_name, "inv") == 0)
            {
                if (!Decode(pIter, x.inv)) return false;
            }
            else
            {
                printf("error member (%s) in GRoleEquipment.\n", child_name);
                return false;
            }
        }

        return true;
    }

    static bool Decode(const tinyxml2::XMLElement* pChild, GRoleStorehouse& x)
    {
        if (pChild == NULL) return false;

        const tinyxml2::XMLElement* pIter = pChild->FirstChildElement();
        for (; pIter != NULL; pIter = pIter->NextSiblingElement())
        {
            const char* child_name = pIter->Name();
            if (child_name == NULL) return false;

            if (strcmp(child_name, "variable") == 0)
            {
                const char* name = pIter->Attribute("name");
                if (name == NULL) return false;
                const char* content = pIter->GetText();
                if (content == NULL) content = "";

                if (strcmp(name, "capacity") == 0)
                    x.capacity = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "money") == 0)
                    x.money = (unsigned int)strtoul(content, 0, 10);
                else if (strcmp(name, "size1") == 0)
                    x.size1 = (unsigned char)strtol(content, 0, 10);
                else if (strcmp(name, "size2") == 0)
                    x.size2 = (unsigned char)strtol(content, 0, 10);
                else if (strcmp(name, "size3") == 0)
                    x.size3 = (unsigned char)strtol(content, 0, 10);
                else if (strcmp(name, "reserved") == 0)
                    x.reserved = (short)strtol(content, 0, 10);
                else
                {
                    printf("error member (%s, %s) in GRoleStorehouse.\n", name, content);
                    return false;
                }
            }
            else if (strcmp(child_name, "items") == 0)
            {
                if (!Decode(pIter, x.items)) return false;
            }
            else if (strcmp(child_name, "dress") == 0)
            {
                if (!Decode(pIter, x.dress)) return false;
            }
            else if (strcmp(child_name, "material") == 0)
            {
                if (!Decode(pIter, x.material)) return false;
            }
            else if (strcmp(child_name, "generalcard") == 0)
            {
                if (!Decode(pIter, x.generalcard)) return false;
            }
            else
            {
                printf("error member (%s) in GRoleStorehouse.\n", child_name);
                return false;
            }
        }

        return true;
    }

    static bool Decode(const tinyxml2::XMLElement* pChild, GRoleTask& x)
    {
        if (pChild == NULL) return false;

        const tinyxml2::XMLElement* pIter = pChild->FirstChildElement();
        for (; pIter != NULL; pIter = pIter->NextSiblingElement())
        {
            const char* child_name = pIter->Name();
            if (child_name == NULL) return false;

            if (strcmp(child_name, "variable") == 0)
            {
                const char* name = pIter->Attribute("name");
                if (name == NULL) return false;
                const char* content = pIter->GetText();
                if (content == NULL) content = "";

                if (strcmp(name, "task_data") == 0)
                    x.task_data = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "task_complete") == 0)
                    x.task_complete = XMLGetOctets(content, strlen(content));
                else if (strcmp(name, "task_finishtime") == 0)
                    x.task_finishtime = XMLGetOctets(content, strlen(content));
                else
                {
                    printf("error member (%s, %s) in GRoleTask.\n", name, content);
                    return false;
                }
            }
            else if (strcmp(child_name, "task_inventory") == 0)
            {
                if (!Decode(pIter, x.task_inventory)) return false;
            }        
            else
            {
                printf("error member (%s) in GRoleTask.\n", child_name);
                return false;
            }
        }

        return true;
    }

    static bool Decode(const tinyxml2::XMLElement* pChild, GRoleForbid& x)
    {
        if (pChild == NULL) return false;

        const char* child_name = pChild->Name();
        if (child_name == NULL) return false;

        if (strcmp(child_name, "variable") == 0)
        {
            const char* name = pChild->Attribute("name");
            if (name == NULL) return false;
            const char* content = pChild->GetText();
            if (content == NULL) content = "";

            if (strcmp(name, "type") == 0)
                x.type = (unsigned char)strtol(content, 0, 10);
            else if (strcmp(name, "time") == 0)
                x.time = (int)strtol(content, 0, 10);
            else if (strcmp(name, "createtime") == 0)
                x.createtime = (int)strtol(content, 0, 10);
            else if (strcmp(name, "reason") == 0)
                x.reason = XMLGetOctets(content, strlen(content));
            else
            {
                printf("error member (%s, %s) in GRoleForbid.\n", name, content);
                return false;
            }
        }
        else
        {
            printf("error member (%s) in GRoleForbid.\n", child_name);
            return false;
        }

        return true;
    }

    static bool Decode(const tinyxml2::XMLElement* pChild, GRoleInventory& x)
    {
        if (pChild == NULL) return false;

        const char* child_name = pChild->Name();
        if (child_name == NULL) return false;

        if (strcmp(child_name, "variable") == 0)
        {
            const char* name = pChild->Attribute("name");
            if (name == NULL) return false;
            const char* content = pChild->GetText();
            if (content == NULL) content = "";

            if (strcmp(name, "id") == 0)
                x.id = (unsigned int)strtoul(content, 0, 10);
            else if (strcmp(name, "pos") == 0)
                x.pos = (int)strtol(content, 0, 10);
            else if (strcmp(name, "count") == 0)
                x.count = (int)strtol(content, 0, 10);
            else if (strcmp(name, "max_count") == 0)
                x.max_count = (int)strtol(content, 0, 10);
            else if (strcmp(name, "data") == 0)
                x.data = XMLGetOctets(content, strlen(content));
            else if (strcmp(name, "proctype") == 0)
                x.proctype = (int)strtol(content, 0, 10);
            else if (strcmp(name, "expire_date") == 0)
                x.expire_date = (int)strtol(content, 0, 10);
            else if (strcmp(name, "guid1") == 0)
                x.guid1 = (int)strtol(content, 0, 10);
            else if (strcmp(name, "guid2") == 0)
                x.guid2 = (int)strtol(content, 0, 10);
            else if (strcmp(name, "mask") == 0)
                x.mask = (int)strtol(content, 0, 10);
            else
            {
                printf("error member (%s, %s) in GRoleInventory.\n", name, content);
                return false;
            }
        }
        else
        {
            printf("error member (%s) in GRoleInventory.\n", child_name);
            return false;
        }

        return true;
    }

    template <typename T>
    static bool Decode(const tinyxml2::XMLElement* pChild, GNET::RpcDataVector<T>& x)
    {
        if (pChild == NULL) return false;
        typename GNET::RpcDataVector<T>::value_type data;

        const tinyxml2::XMLElement* pIter = pChild->FirstChildElement();
        for (; pIter != 0; pIter = pIter->NextSiblingElement())
        {
            if (!Decode(pIter, data)) return false;
        }

        x.push_back(data);
        return true;
    }

};


}

#endif

