// ============================================================
// TX Engine — Technologic Experience Engine
// Blueprint System (TXB)
// ============================================================

#include <vector>
#include <unordered_map>
#include <functional>
#include <string>
#include <cstdint>
#include <cassert>

// ------------------------------------------------------------
// Tipos base
// ------------------------------------------------------------

using TXNodeID = uint32_t;
using TXPinID  = uint16_t;
using TXOpCode = uint8_t;

// ------------------------------------------------------------
// Contexto de ejecución (por entidad / actor)
// ------------------------------------------------------------

struct TXExecutionContext
{
    float DeltaTime;
    void* UserData;   // puntero a Actor, Entity, etc.
};

// ------------------------------------------------------------
// Tipos de valores soportados
// ------------------------------------------------------------

enum class TXValueType : uint8_t
{
    None,
    Int,
    Float,
    Bool,
    Vector3,
    Pointer
};

struct TXValue
{
    TXValueType Type = TXValueType::None;

    union
    {
        int     Int;
        float   Float;
        bool    Bool;
        void*   Ptr;
    };
};

// ------------------------------------------------------------
// Nodo Blueprint (definición estática)
// ------------------------------------------------------------

struct TXBlueprintNode
{
    TXNodeID ID;
    TXOpCode OpCode;
    uint16_t InputCount;
    uint16_t OutputCount;
};

// ------------------------------------------------------------
// Bytecode Blueprint
// ------------------------------------------------------------

struct TXBlueprintBytecode
{
    std::vector<TXOpCode> Instructions;
    std::vector<TXValue> Constants;
};

// ------------------------------------------------------------
// Definición de una función de nodo
// ------------------------------------------------------------

using TXNodeFunction = std::function<void(
    TXExecutionContext&,
    const TXValue* Inputs,
    TXValue* Outputs
)>;

// ------------------------------------------------------------
// Registro global de nodos
// ------------------------------------------------------------

class TXNodeRegistry
{
public:
    static TXNodeRegistry& Instance()
    {
        static TXNodeRegistry Registry;
        return Registry;
    }

    void RegisterNode(TXOpCode opcode, TXNodeFunction fn)
    {
        NodeFunctions[opcode] = fn;
    }

    TXNodeFunction& Get(TXOpCode opcode)
    {
        assert(NodeFunctions.count(opcode));
        return NodeFunctions[opcode];
    }

private:
    std::unordered_map<TXOpCode, TXNodeFunction> NodeFunctions;
};

// ------------------------------------------------------------
// Máquina virtual Blueprint
// ------------------------------------------------------------

class TXBlueprintVM
{
public:
    void Execute(
        const TXBlueprintBytecode& Bytecode,
        TXExecutionContext& Context)
    {
        const TXOpCode* IP = Bytecode.Instructions.data();
        const TXOpCode* End = IP + Bytecode.Instructions.size();

        TXValue Stack[64];   // stack fijo = cache friendly
        int SP = 0;

        while (IP < End)
        {
            TXOpCode Op = *IP++;

            TXNodeFunction& Fn = TXNodeRegistry::Instance().Get(Op);

            // Por simplicidad: 2 inputs, 1 output
            Fn(Context, &Stack[SP - 2], &Stack[SP - 2]);

            SP--; // consumir inputs -> producir output
        }
    }
};

// ------------------------------------------------------------
// Nodos básicos
// ------------------------------------------------------------

enum : TXOpCode
{
    TX_OP_ADD_FLOAT = 1,
    TX_OP_MUL_FLOAT,
    TX_OP_PRINT_FLOAT
};

void RegisterCoreNodes()
{
    TXNodeRegistry::Instance().RegisterNode(
        TX_OP_ADD_FLOAT,
        [](TXExecutionContext&, const TXValue* In, TXValue* Out)
        {
            Out->Type = TXValueType::Float;
            Out->Float = In[0].Float + In[1].Float;
        });

    TXNodeRegistry::Instance().RegisterNode(
        TX_OP_MUL_FLOAT,
        [](TXExecutionContext&, const TXValue* In, TXValue* Out)
        {
            Out->Type = TXValueType::Float;
            Out->Float = In[0].Float * In[1].Float;
        });

    TXNodeRegistry::Instance().RegisterNode(
        TX_OP_PRINT_FLOAT,
        [](TXExecutionContext&, const TXValue* In, TXValue*)
        {
            printf("[TXB] %f\n", In[0].Float);
        });
}

// ------------------------------------------------------------
// Ejemplo de uso
// ------------------------------------------------------------

void ExampleBlueprint()
{
    RegisterCoreNodes();

    TXBlueprintBytecode BP;

    // Simula: print( (2.0 + 3.0) * 4.0 )

    BP.Instructions = {
        TX_OP_ADD_FLOAT,
        TX_OP_MUL_FLOAT,
        TX_OP_PRINT_FLOAT
    };

    TXExecutionContext Ctx;
    Ctx.DeltaTime = 0.016f;
    Ctx.UserData = nullptr;

    TXBlueprintVM VM;
    VM.Execute(BP, Ctx);
}