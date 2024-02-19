#ifndef FUZZYDROPLETS_GUI_GENERIC_COMMAND_H
#define FUZZYDROPLETS_GUI_GENERIC_COMMAND_H

class BaseCommand
{
    template <typename T> friend class Command;

public:

    virtual ~BaseCommand() {}
    virtual int id() const = 0;
    virtual void redo() = 0;
    virtual void undo() = 0;
    virtual bool compressInto(BaseCommand *) const {return false;}

private:

    static int m_typeCounter;
};

template <typename T>
class Command : public BaseCommand
{
public:

    virtual ~Command() {}
    int id() const final {return guid;}
    static int ID() {return guid;}

private:

    static const int guid;
};

template <typename T> const int Command<T>::guid = m_typeCounter++;

#endif // FUZZYDROPLETS_GUI_GENERIC_COMMAND_H
