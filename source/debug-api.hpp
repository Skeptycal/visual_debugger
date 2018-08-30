#include "utils.h"
#include "io-broadcast.hpp"

using namespace Halide;

// from 'imgui_main.cpp':
extern bool stdout_echo_toggle;
void run_gui(std::vector<Func> funcs, Halide::Buffer<> output_buff);

struct UI
{
    bool running = false;
    void run(std::vector<Func> funcs, Halide::Buffer<> output)
    {
        running = true;
        // redirect stdout to a log file, effectivelly silencing the console output:
        const char* logfile = "data/output/log-halide-visdbg.txt";
        fprintf(stdout, ">> Redirecting 'stdout' to log file '%s'\n", logfile);
        FILE* log = fopen(logfile, "w");
        assert(log);
        Broadcaster iobc = redirect_broadcast(stdout);
        iobc.AddEcho(&stdout_echo_toggle);
        iobc.AddFile(log);
        
        run_gui(funcs, output);
        running = false;
        
        iobc.Terminate();
        fprintf(stdout, "<< Done with 'stdout' redirection\n");
        
        fclose(log);
    }
    
    
};

struct DebugFunc
{
    Func f;
    Halide::Buffer<> output;
    
    void realize(Pipeline::RealizationArg outputs, const Target &target = Target(), const ParamMap & param_map = ParamMap::empty_map()) //args, ...
    {
        if(outputs.size() == 1)
        {
            halide_buffer_t * buf = outputs.buf;
            Buffer<> buffer (*buf);
            this->output = buffer;
        }
        else
        {
            this->output = outputs.buffer_list->at(0);
        }
        UI ui;
        std::vector<Func> funcs;
        funcs.push_back(this->f);
        
        ui.run(funcs, this->output);
        while(ui.running){
            //gui running
        }
        this->f.realize(std::move(outputs), target, param_map);
        
    }
    
    //TODO(Emily): other realize calls
    Realization realize(std::vector<int32_t> sizes, const Target &target = Target(),
                        const ParamMap &param_map = ParamMap::empty_map())
    {
        UI ui;
        std::vector<Func> funcs;
        funcs.push_back(this->f);
        Halide::Buffer<> output = this->f.realize(sizes, target, param_map); //TODO(Emily): need to handle case of multiple outputs
        this->output = std::move(output);
        ui.run(funcs, output);
        while(ui.running)
        {
            //gui running
        }
        return this->output;
    }
    
    Realization realize(int x_size, int y_size, int z_size, int w_size, const Target &target = Target(),
                        const ParamMap &param_map = ParamMap::empty_map())
    {
        UI ui;
        std::vector<Func> funcs;
        funcs.push_back(this->f);
        Halide::Buffer<> output = this->f.realize(x_size, y_size, z_size, w_size, target, param_map); //TODO(Emily): need to handle case of multiple outputs
        this->output = std::move(output);
        ui.run(funcs, output);
        while(ui.running)
        {
            //gui running
        }
        return this->output;
    }
    
    Realization realize(int x_size, int y_size, int z_size, const Target &target = Target(),
                        const ParamMap &param_map = ParamMap::empty_map())
    {
        UI ui;
        std::vector<Func> funcs;
        funcs.push_back(this->f);
        Halide::Buffer<> output = this->f.realize(x_size, y_size, z_size, target, param_map); //TODO(Emily): need to handle case of multiple outputs
        this->output = std::move(output);
        ui.run(funcs, output);
        while(ui.running)
        {
            //gui running
        }
        return this->output;
    }
    
    Realization realize(int x_size, int y_size, const Target &target = Target(),
                        const ParamMap &param_map = ParamMap::empty_map())
    {
        UI ui;
        std::vector<Func> funcs;
        funcs.push_back(this->f);
        Halide::Buffer<> output = this->f.realize(x_size, y_size, target, param_map); //TODO(Emily): need to handle case of multiple outputs
        this->output = std::move(output);
        ui.run(funcs, output);
        while(ui.running)
        {
            //gui running
        }
        return this->output;
    }
    
    Realization realize(Halide::Buffer<> input, int x_size, const Target &target = Target(),
                        const ParamMap &param_map = ParamMap::empty_map())
    {
        UI ui;
        std::vector<Func> funcs;
        funcs.push_back(this->f);
        Halide::Buffer<> output = this->f.realize(x_size, target, param_map); //TODO(Emily): need to handle case of multiple outputs
        this->output = std::move(output);
        ui.run(funcs, output);
        while(ui.running)
        {
            //gui running
        }
        return this->output;
    }
    
    Realization realize(const Target &target = Target(),
                        const ParamMap &param_map = ParamMap::empty_map())
    {
        
        UI ui;
        std::vector<Func> funcs;
        funcs.push_back(this->f);
        Halide::Buffer<> output = this->f.realize(target, param_map); //TODO(Emily): need to handle case of multiple outputs
        this->output = std::move(output);
        ui.run(funcs, output);
        while(ui.running)
        {
            //gui running
        }
        return this->output;
    }
};

DebugFunc debug(Func f)
{
    DebugFunc debug_f;
    debug_f.f = f;
    return debug_f;
}
