/*
 * (c) 2011 Thomas
 *
 */

namespace e
{
    bool loadents(const char *mname, vector<entity> &ents, uint &mapcrc)
    {
        defformatstring(mf)("mapinfo/%s.ents", mname);
        stream *f = opengzfile(path(mf), "rb");
        if (!f) return false;

        if (f->getchar() != 'M' || f->getchar() != 'A' || f->getchar() != 'P' ||
            f->getchar() != 'E' || f->getchar() != 'N' || f->getchar() != 'T' || f->getchar() != 'S')
        {
            delete f;
            return false;
        }

        mapcrc = f->get<uint>();
        int elen = f->get<int>();

        if (f->get<int>() != 0)
        {
            delete f;
            return false;
        }

        for(int i = 0; i < elen; i++)
        {
            entity e;
            e.type = f->get<uchar>();
            e.attr1 = f->get<short>();
            e.attr2 = f->get<short>();
            e.attr3 = f->get<short>();
            e.attr4 = f->get<short>();
            e.attr5 = f->get<short>();
            e.reserved = f->get<uchar>();
            loopk(3) e.o[k] = f->get<float>();

            ents.add(e);

            if (f->getlil<int>() != 0) 
            {
                ents.shrink(0);
                delete f;
                return false;
            }
        }

        if (f->get<int>() != 0 || f->get<int>() != elen)
        {
            ents.shrink(0);
            delete f;
            return false;
        }

        delete f;
        return true;
    }

    bool writeents(const char *mapname, vector<entity> &ents, uint mapcrc)
    {
        string file;
        formatstring(file)("mapinfo/%s.ents", mapname);

        stream *mapi = opengzfile(path(file), "w+b");

        if (!mapi) return false;

        mapi->putstring("MAPENTS");
        mapi->put(mapcrc);
        mapi->put(ents.length());
        mapi->put(0);

        loopv(ents)
        {  
            entity &e = ents[i];

            mapi->put(e.type);
            mapi->put(e.attr1);
            mapi->put(e.attr2);
            mapi->put(e.attr3);
            mapi->put(e.attr4);
            mapi->put(e.attr5);
            mapi->put(e.reserved);
            loopk(3) mapi->put(e.o[k]);

            mapi->putlil(0);
        }

        mapi->put(0);
        mapi->put(ents.length());

        mapi->close();
        delete mapi;
        return true;
    }
}
