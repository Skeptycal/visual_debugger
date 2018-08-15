// ImGui - standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See imgui_impl_glfw.cpp for details.

// include our own local copy of imconfig.h, should we need to customize it
#include "imconfig.h"
// we also need to define this to prevent imgui.h form including the stock
// version of imconfig.h
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

#include "treedump.cpp"

#include "io-broadcast.hpp"

bool stdout_echo_toggle (false);
bool save_images(false);

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct rgba {
    float r, g, b, a;
};


//NOTE(Emily): method to generate example tree using struc
expr_node * generate_example_tree(){
    expr_node * root = new expr_node();
    root->name = "Func: output";
    expr_node * temp = new expr_node();
    temp->name = "<arguments>";
    root->children.push_back(temp);
    temp->name = "Variable: v60 | let";
    root->children[0]->children.push_back(temp);
    temp->name = "Variable: v61 | let";
    root->children[0]->children.push_back(temp);
    temp->name = "Variable: v62 | let";
    root->children[0]->children.push_back(temp);
    temp->name = "<definition>";
    root->children.push_back(temp);
    temp->name = "Function: f";
    root->children[1]->children.push_back(temp);
    temp->name = "<arguments>";
    root->children[1]->children[0]->children.push_back(temp);
    temp->name = "Variable: v54 | let";
    root->children[1]->children[0]->children[0]->children.push_back(temp);
    temp->name = "Variable: v55 | let";
    root->children[1]->children[0]->children[0]->children.push_back(temp);
    temp->name = "Variable: v56 | let";
    root->children[1]->children[0]->children[0]->children.push_back(temp);
    temp->name = "<definition>";
    root->children[1]->children[0]->children.push_back(temp);
    temp->name = "Mul";
    root->children[1]->children[0]->children[1]->children.push_back(temp);
    temp->name = "UIntImm: 10";
    root->children[1]->children[0]->children[1]->children[0]->children.push_back(temp);
    temp->name = "Call: image | Halide";
    root->children[1]->children[0]->children[1]->children[0]->children.push_back(temp);
    temp->name = "...";
    root->children[1]->children[0]->children[1]->children[0]->children[1]->children.push_back(temp);
    return root;
}

void set_color(std::vector<rgba>& pixels){
    float R = rand()/(float)RAND_MAX;
    float G = rand()/(float)RAND_MAX;
    float B = rand()/(float)RAND_MAX;
    for(auto& p : pixels){
        p.r = R;
        p.g = G;
        p.b = B;
        p.a = 1;
    }
}

void update_buffer(GLuint idMyTexture, std::vector<rgba> pixels, int width, int height){
    glBindTexture(GL_TEXTURE_2D, idMyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ToggleButton(const char* str_id, bool* v)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;
    
    if (ImGui::InvisibleButton(str_id, ImVec2(width, height)))
        *v = !*v;
    ImU32 col_bg;
    if (ImGui::IsItemHovered())
        col_bg = *v ? IM_COL32(145+20, 211, 68+20, 255) : IM_COL32(218-20, 218-20, 218-20, 255);
    else
        col_bg = *v ? IM_COL32(145, 211, 68, 255) : IM_COL32(218, 218, 218, 255);
    
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

int id_expr_debugging = -1;
Halide::Type selected_type;

void display_node(expr_node * parent, GLuint idMyTexture, int width, int height, Func f, Halide::Buffer<uint8_t>& input_full, std::string& selected_name, Profiling& times, const std::string& target_features)
{
    if (id_expr_debugging == parent->node_id)
    {
        ImGui::PushStyleColor(ImGuiCol_Text,   0xFF00CF40);
        ImGui::PushStyleColor(ImGuiCol_Button, 0xFF00CF40);
    }

    bool clicked = false;
    if (parent->node_id != 0)
    {
        ImGui::PushID(parent->node_id);
        clicked = ImGui::SmallButton(" ");
        ImGui::PopID();
        ImGui::SameLine();
    }

    bool open = false;
    if (parent->children.empty())
    {
        ImGui::Text(parent->name.c_str());
    }
    else
    {
        open = ImGui::TreeNode(parent->name.c_str());
    }

    if (id_expr_debugging == parent->node_id)
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }

    if (clicked)
    {
        selected_name = parent->name;
        times = select_and_visualize(f, parent->node_id, input_full, idMyTexture, target_features, save_images);
        id_expr_debugging = parent->node_id;
        selected_type = parent->original.type();
    }

    if (!open)
    {
        return;
    }

    if(!parent->children.empty())
    {
        for(int i = 0; i < parent->children.size(); i++)
        {
            display_node(parent->children[i], idMyTexture, width, height, f, input_full, selected_name, times, target_features);
        }
    }

    ImGui::TreePop();
}

std::string type_to_string(Halide::Type type)
{
    std::stringstream ss;
    switch (type.code())
    {
        case halide_type_uint:
            ss << "u";
        case halide_type_int:
            ss << "int";
            break;
        case halide_type_float:
            ss << "float";
            break;
        case halide_type_handle:
            ss << "handle";
            break;
    }
    ss << type.bits();
    ss << "[";
    ss << type.lanes();
    ss << "]";
    return ss.str();
}

void render_gui(GLFWwindow* window)
{
    ImGui::Render();
    //int display_w, display_h;
    //glfwGetFramebufferSize(window, &display_w, &display_h);
    //glViewport(0, 0, display_w, display_h);
    //glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    //glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

void glfw_on_window_resized(GLFWwindow* window, int width, int height)
{
    assert(glfwGetCurrentContext() == window);
    glViewport(0, 0, width, height);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    render_gui(window);
}


// FILE SYSTEM USAGE EXAMPLE:
/*
 #include "imguifilesystem.h"                                                    // imguifilesystem.cpp must be compiled
 // Inside a ImGui window:
 const bool browseButtonPressed = ImGui::Button("...");                          // we need a trigger boolean variable
 static ImGuiFs::Dialog dlg;                                                     // one per dialog (and must be static)
 const char* chosenPath = dlg.chooseFileDialog(browseButtonPressed);             // see other dialog types and the full list of arguments for advanced usage
 if (strlen(chosenPath)>0) {
 // A path (chosenPath) has been chosen RIGHT NOW. However we can retrieve it later more comfortably using: dlg.getChosenPath()
 }
 if (strlen(dlg.getChosenPath())>0) {
 ImGui::Text("Chosen file: \"%s\"",dlg.getChosenPath());
 }
 // If you want to copy the (valid) returned path somewhere, you can use something like:
 static char myPath[ImGuiFs::MAX_PATH_BYTES];
 if (strlen(dlg.getChosenPath())>0) {
 strcpy(myPath,dlg.getChosenPath());
 }
 */

std::string save_filename = "data/output/test.png";
int n(0);
void file_system_popup()
{
    ImGui::OpenPopup("Save Image");
    const bool popup_ok = ImGui::BeginPopupModal("Save Image");
    if(!popup_ok) return;
    
    ImGui::Text("Here is the popup");
    
    n++;
    //std::string filename = "data/output/test.png";
    save_filename = "data/output/test" + std::to_string(n) + ".png";
    //ImGui::CloseCurrentPopup();
    
    ImGui::EndPopup();
}

void run_gui(std::vector<Func> funcs, Halide::Buffer<uint8_t>& input_full, Broadcaster iobc)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Halide Visual Debugger", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= 0; // this no-op is here just to suppress unused variable warnings
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();
    //glfwSetWindowSizeCallback(window, glfw_on_window_resized);
    glfwSetFramebufferSizeCallback(window, glfw_on_window_resized);

    // Setup style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    //bool show_another_window = true;
    bool show_image = true;
    bool show_expr_tree = true;
    bool show_func_select = true;
    bool show_target_select = true;
    bool show_stdout_box = true;
    bool show_save_image = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
    std::string selected_name = "No node selected, displaying output";
    
    
    int width = input_full.width();
    int height = input_full.height();
    int channels = input_full.channels();

    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    
    GLuint idMyTexture = 0;
    glGenTextures(1, &idMyTexture);
    assert(idMyTexture != 0);
    
    glBindTexture(GL_TEXTURE_2D, idMyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // TODO(marcos): maybe we should make 'select_and_visualize' return the
    // corresponding expr_tree; this will spare us of a visitor step.
    //expr_tree tree = get_tree(funcs[0]);
    expr_tree tree;
    std::string target_features = "";
        //"fma"     // FMA is actually orthogonal to AVX (and is even orthogonal to AVX2!)
        //"fma4"    // FMA4 is AMD-only; Intel adopted FMA3 (which Halide does not yet support)
        //"avx-sse41"
        //"avx-avx2-sse41"
        //"sse41"
        //"cuda"
        //"metal"
        //"opencl"
        //"d3d12"
    ;

    //NOTE(Emily): call to update buffer to display output of function
    //Profiling times = select_and_visualize(f, 0, input_full, idMyTexture, target_features);
    Profiling times = { };
    int cpu_value(0), gpu_value(0), func_value(0);
    


    //target flag bools (need to be outside of loop to maintain state)
    bool sse41(false), avx(false), avx2(false), avx512(false), fma(false), fma4(false);
    bool neon(false);
    bool debug_runtime(false), no_asserts(false), no_bounds_query(false);
    bool save_current(false);
    //NOTE(Emily): temporary to explore demo window
    bool open_demo(false);

    Func selected;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Start the ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if(show_stdout_box)
        {
            bool * no_close = NULL;
            ImGui::Begin("Output", no_close);
            ImGui::Checkbox("Show stdout in terminal", &stdout_echo_toggle);
            ImGui::End();
        }
        
        if(true)
        {
            ImGui::Begin("display demo window");
            ImGui::Checkbox("open", &open_demo);
            if(open_demo)
            {
                ImGui::ShowDemoWindow();
            }
            ImGui::End();
        }
        
        if(show_save_image)
        {
            bool * no_close = NULL;
            ImGui::Begin("Save images to disk upon selection: ", no_close);
            ToggleButton("Save all images", &save_images);
            ImGui::SameLine();
            ImGui::Text("Save all displayed images");
            if(!save_images)
            {
                //allow user to select individual images to save
                ImGui::Checkbox("Save current image:" , &save_current);
            }
            if(save_current)
            {
                file_system_popup();
                times = select_and_visualize(selected, id_expr_debugging, input_full, idMyTexture, target_features, save_current);
                save_current = false; //NOTE(Emily): want to "uncheck" the box after saving
                
            }
            ImGui::End();
        }
        
        std::string target_features_before = target_features;

        if(show_target_select)
        {
            bool * no_close = NULL;
            
            ImGui::Begin("Select compilation target: ", no_close);
            
            ImGui::Text("CPU: ");

            ImGui::RadioButton("host", &cpu_value, 0);
            if (cpu_value == 0) target_features = "host";

            ImGui::RadioButton("x86", &cpu_value, 1);
            if(cpu_value == 1) target_features = "x86";
            ImGui::SameLine();
            ImGui::RadioButton("x86_64", &cpu_value, 2);
            if(cpu_value == 2) target_features = "x86_64";

            ImGui::RadioButton("ARM", &cpu_value, 3);
            if(cpu_value == 3) target_features = "armv7s";

            if(cpu_value == 1 || cpu_value == 2)
            {
                ImGui::Text("CPU: x86/x64 options");
                
                ImGui::Checkbox("sse41", &sse41);
                if(sse41) target_features += "-sse41";
                ImGui::SameLine();
                ImGui::Checkbox("avx", &avx);
                if(avx) target_features += "-avx";
                ImGui::SameLine();
                ImGui::Checkbox("avx2", &avx2);
                if(avx2) target_features += "-avx2";
                ImGui::SameLine();
                ImGui::Checkbox("avx512", &avx512);
                if(avx512) target_features += "-avx512";

                ImGui::Checkbox("fma", &fma);
                if(fma) target_features += "-fma";
                ImGui::SameLine();
                ImGui::Checkbox("fma4", &fma4);
                if(fma4) target_features += "-fma4";
            }

            if(cpu_value == 3)
            {
                
                ImGui::Text("CPU: ARM Options");
                ImGui::Checkbox("NEON", &neon);
                if(neon) target_features += "-neon";
                else target_features += "-no_neon";
            }
            
            ImGui::Text("GPU: ");
            
            ImGui::RadioButton("none", &gpu_value, 0);
            ImGui::RadioButton("Metal", &gpu_value, 1);
            if(gpu_value == 1) target_features += "-metal";
            ImGui::RadioButton("CUDA", &gpu_value, 2);
            if(gpu_value == 2) target_features += "-cuda";
            ImGui::RadioButton("OpenCL", &gpu_value, 3);
            if(gpu_value == 3) target_features += "-opencl";
            ImGui::RadioButton("Direct3D 12", &gpu_value, 4);
            if(gpu_value == 4) target_features += "-d3d12compute";

            ImGui::Text("Halide: ");
            ImGui::Checkbox("Debug Runtime", &debug_runtime);
            if(debug_runtime) target_features += "-debug";
            ImGui::Checkbox("No Asserts", &no_asserts);
            if(no_asserts) target_features += "-no_asserts";
            ImGui::Checkbox("No Bounds Query", &no_bounds_query);
            if(no_bounds_query) target_features += "-no_bounds_query";

            //ImGui::Text("%s", target_features.c_str());

            ImGui::End();
            
        }

        bool target_changed = (target_features_before != target_features);
        target_features_before = target_features;

        bool target_selected = !target_features.empty();

        if(show_func_select)
        {
            bool * no_close = NULL;
            ImGui::Begin("Select Func to visualize: ", no_close);
            int id = 0;
            for(auto func : funcs)
            {
                int func_value_before = func_value;
                ImGui::RadioButton(func.name().c_str(), &func_value, id);
                bool changed = (func_value_before != func_value) || !selected.defined() || target_changed;
                if(func_value == id && changed)
                {
                    tree = get_tree(func);
                    times = select_and_visualize(func, 0, input_full, idMyTexture, target_features, save_images);
                    selected = func;
                    selected_type = func.output_types()[0]; //NOTE(Emily): need to handle case with multiple outputs or update definitions
                    id_expr_debugging = -1;
                    break;
                }
                id++;
            }
            ImGui::End();
        }

        bool func_selected = selected.defined();
        

        // NOTE(Emily): main expression tree window
        if(show_expr_tree)
        {
            
            bool * no_close = NULL;
            //ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            //ImGui::SetNextWindowSize(ImVec2(500,600));
            ImGui::Begin("Expression Tree", no_close, ImGuiWindowFlags_HorizontalScrollbar);
            //Note(Emily): call recursive method to display tree
            if(func_selected && target_selected)
            {
                display_node(tree.root, idMyTexture, width, height, selected, input_full, selected_name, times, target_features);
            }
            ImGui::End();
            
        }
        
        
        //NOTE(Emily): Window to show image info - making more compact and putting info as title of image box
        /*
        if (show_another_window)
        {
            
            bool * no_close = NULL;
            //ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
            //ImGui::SetNextWindowSize(ImVec2(300,100));
            ImGui::Begin("Image Information", no_close);
            ImGui::Text("Information about the currently displayed image: ");
            std::string size_info = "width: " + std::to_string(width) + " height: " + std::to_string(height) + " channels: " + std::to_string(channels);
            ImGui::Text("%s", size_info.c_str());
            ImGui::Text("Type of selected expression: %s", type_to_string(selected_type).c_str());
            ImGui::Text("Time for JIT compilation: %f", times.jit_time);
            ImGui::Text("Time to realize: %f", times.run_time);
            ImGui::End();
        }
        */
        
        if (show_image)
        {
            bool * no_close = NULL;
            
            std::string info = std::to_string(width) + "x" + std::to_string(height)
                            + "x" + std::to_string(channels) + " | " + type_to_string(selected_type)
                            + " | " + std::to_string(times.run_time) + "s | (jit: "
                            + std::to_string(times.jit_time) + "s)"
                            + "###ImageDisplay";   // <- ImGui ID control (###): we want this window to be the same, regardless of its title
            
            //ImGui::SetNextWindowPos(ImVec2(650, 200), ImGuiCond_FirstUseEver);
            //ImGui::SetNextWindowSize(ImVec2(500,500));
            ImGui::Begin(info.c_str() , no_close, ImGuiWindowFlags_HorizontalScrollbar);
            
            static float zoom = 1.0f;
            ImGui::SliderFloat("Image Zoom", &zoom, 0, 10, "%.001f");
            
            ImGui::BeginChild(" ", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar); //NOTE(Emily): in order to get horizontal scrolling needed to add other parameters (from example online)
                ImGui::Image((void *) (uintptr_t) idMyTexture , ImVec2(width*zoom, height*zoom));
            ImGui::EndChild();
            ImGui::End();
        }

        // Rendering
        render_gui(window);

        // NOTE(marcos): moving event handling to the end of the main loop so
        // that we don't "lose" the very first frame and are left staring at a
        // blank screen until the mouse starts hovering the window:
        // (alternatively, we can use 'glfwPostEmptyEvent()' to force an event
        //  artificially into the event queue)

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        //glfwPollEvents();     // <- this is power hungry!
        glfwWaitEvents();       // <- this is a more CPU/power/battery friendly choice
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

}
