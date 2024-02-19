#ifndef FUZZYDROPLETS_GUI_GENERIC_COMMANDSTACK_H
#define FUZZYDROPLETS_GUI_GENERIC_COMMANDSTACK_H

#include "command.h"

#include <QObject>
#include <memory>

class BaseCommand;

class BeginCommandSet : public Command<BeginCommandSet>
{
public:

    void redo() {}
    void undo() {}
    bool compressInto(BaseCommand *) const;
};

class EndCommandSet : public Command<EndCommandSet>
{
public:

    void redo() {}
    void undo() {}
    bool compressInto(BaseCommand *) const;
};

class CommandStack : public QObject
{
    Q_OBJECT

public:

    CommandStack(QObject * parent = nullptr);

    void beginCommandSet();
    void add(std::shared_ptr<BaseCommand> cmd, bool doIt = true);
    void add(BaseCommand * cmd, bool doIt = true);
    void add(QList<std::shared_ptr<BaseCommand>> & cmd, bool doIt = true);
    void add(QList<BaseCommand *> & cmd, bool doIt = true);
    void endCommandSet();
    bool isInCommandSet() const {return m_inSet;}

    void clearUndone();

    bool canRedo() const {return (m_pos < m_commands.size() - 1);}
    bool canUndo() const {return (m_pos >= 0);}

    int size() const {return m_commands.count();}

public slots:

    void redo();
    void undo();

signals:

    void undoAvailable(bool);
    void redoAvailable(bool);

private:

    int m_pos {-1};
    QList<std::shared_ptr<BaseCommand>> m_commands;
    bool m_newCommandsBlocked {false};
    bool m_inSet {false};
};

#endif // FUZZYDROPLETS_GUI_GENERIC_COMMANDSTACK_H
