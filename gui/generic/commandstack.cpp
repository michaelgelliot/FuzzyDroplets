#include "commandstack.h"
#include "command.h"

bool BeginCommandSet::compressInto(BaseCommand * cmd) const
{
    return cmd->id() == id();
}

bool EndCommandSet::compressInto(BaseCommand * cmd) const
{
    return cmd->id() == id();
}

CommandStack::CommandStack(QObject * parent)
    : QObject(parent)
{
}

void CommandStack::beginCommandSet()
{
    if (!m_newCommandsBlocked && !m_inSet) {
        m_inSet = true;
        add(std::shared_ptr<BaseCommand>(new BeginCommandSet));
    }
}

void CommandStack::endCommandSet()
{
    if (!m_newCommandsBlocked && m_inSet) {
        bool u = canUndo();
        bool r = canRedo();
        m_inSet = false;
        if (m_commands[m_pos]->id() == BeginCommandSet::ID()) {
            --m_pos;
            clearUndone();
        } else {
            add(std::shared_ptr<BaseCommand>(new EndCommandSet));
        }
        if (canUndo() != u) emit undoAvailable(canUndo());
        if (canRedo() != r) emit redoAvailable(canRedo());
    }
}

void CommandStack::clearUndone()
{
    if (m_pos < m_commands.size()-1) {
        m_commands.remove(m_pos + 1, m_commands.size() - (m_pos + 1));
    }
    emit redoAvailable(false);
}

void CommandStack::add(QList<BaseCommand*> & cmd, bool doIt)
{
    for (auto & c : cmd) {
        add(std::shared_ptr<BaseCommand>(c), doIt);
    }
}

void CommandStack::add(QList<std::shared_ptr<BaseCommand>> & cmd, bool doIt)
{
    for (auto & c : cmd) {
        add(c, doIt);
    }
}

void CommandStack::add(BaseCommand * cmd, bool doIt)
{
    add(std::shared_ptr<BaseCommand>(cmd), doIt);
}

void CommandStack::add(std::shared_ptr<BaseCommand> cmd, bool doIt)
{
    if (!cmd)
        return;

    if (doIt) {
        cmd->redo();
    }

    if (m_newCommandsBlocked) return;

    bool u = canUndo();
    bool r = canRedo();

    clearUndone();

    if (m_commands.empty() || !m_inSet || !cmd->compressInto(m_commands.back().get())) {
        m_commands.append(cmd);
        ++m_pos;
    }

    if (canUndo() != u) emit undoAvailable(canUndo());
    if (canRedo() != r) emit redoAvailable(canRedo());
}

void CommandStack::redo()
{
    bool r = canRedo();
    if (!r) return;
    bool u = canUndo();

    bool b = m_newCommandsBlocked;
    m_newCommandsBlocked = true;

    ++m_pos;

    if (m_commands[m_pos]->id() == BeginCommandSet::ID()) {
        ++m_pos;
        while (m_commands[m_pos]->id() != EndCommandSet::ID() && m_pos < m_commands.size()) {
            m_commands[m_pos]->redo();
            ++m_pos;
        }
    } else {
        m_commands[m_pos]->redo();
    }
    if (m_pos == m_commands.size()) {
        --m_pos;
    }

    m_newCommandsBlocked = b;

    if (canUndo() != u) emit undoAvailable(canUndo());
    if (canRedo() != r) emit redoAvailable(canRedo());
}

void CommandStack::undo()
{
    if (m_inSet) endCommandSet();

    bool u = canUndo();
    if (!u) return;
    bool r = canRedo();

    bool b = m_newCommandsBlocked;
    m_newCommandsBlocked = true;

    if (m_commands.at(m_pos)->id() == EndCommandSet::ID()) {
        -- m_pos;
        while (m_pos >= 0 && m_commands[m_pos]->id() != BeginCommandSet::ID()) {
            m_commands[m_pos]->undo();
            --m_pos;
        }
        --m_pos;
    } else {
        m_commands[m_pos]->undo();
        --m_pos;
    }

    m_newCommandsBlocked = b;

    if (canUndo() != u) emit undoAvailable(canUndo());
    if (canRedo() != r) emit redoAvailable(canRedo());
}

