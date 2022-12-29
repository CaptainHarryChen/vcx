#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <spdlog/spdlog.h>
#include <iostream>
#include "Labs/4-Animation/tasks.h"
#include "IKSystem.h"
#include "CustomFunc.inl"
#include <stb_image.h>
#include <stb_image_write.h>


namespace VCX::Labs::Animation {
    void ForwardKinematics(IKSystem & ik, int StartIndex) {
        if (StartIndex == 0) {
            ik.JointGlobalRotation[0] = ik.JointLocalRotation[0];
            ik.JointGlobalPosition[0] = ik.JointLocalOffset[0];
            StartIndex                = 1;
        }
        
        for (int i = StartIndex; i < ik.JointLocalOffset.size(); i++) {
            // your code here: forward kinematics
            ik.JointGlobalRotation[i] = ik.JointGlobalRotation[i - 1] * ik.JointLocalRotation[i];
            glm::vec4 pos = glm::vec4(ik.JointLocalOffset[i], 1.0f);
            pos = glm::mat4_cast(ik.JointGlobalRotation[i - 1]) * pos;
            ik.JointGlobalPosition[i] = ik.JointGlobalPosition[i - 1] + glm::vec3(pos.x, pos.y, pos.z) / pos.w;
        }
    }

    void InverseKinematicsCCD(IKSystem & ik, const glm::vec3 & EndPosition, int maxCCDIKIteration, float eps) {
        ForwardKinematics(ik, 0);
        // These functions will be useful: glm::normalize, glm::rotation, glm::quat * glm::quat
        int CCDIKIteration;
        for (CCDIKIteration = 0; CCDIKIteration < maxCCDIKIteration && glm::l2Norm(ik.EndEffectorPosition() - EndPosition) > eps; CCDIKIteration++) {
            // your code here: ccd ik
            int n = ik.NumJoints();
            for(int i = n - 2; i >= 0; i--) {
                glm::vec3 &st = ik.JointGlobalPosition[i];
                glm::vec3 orig = glm::normalize(ik.EndEffectorPosition() - st);
                glm::vec3 dest = glm::normalize(EndPosition - st);
                glm::quat rot = glm::rotation(orig, dest);
                ik.JointLocalRotation[i] = rot * ik.JointLocalRotation[i];
                ForwardKinematics(ik, i);
            }
        }
        //printf("%d\n", CCDIKIteration);
    }

    void InverseKinematicsFABR(IKSystem & ik, const glm::vec3 & EndPosition, int maxFABRIKIteration, float eps) {
        ForwardKinematics(ik, 0);
        int nJoints = ik.NumJoints();
        std::vector<glm::vec3> backward_positions(nJoints, glm::vec3(0, 0, 0)), forward_positions(nJoints, glm::vec3(0, 0, 0));
        int IKIteration;
        for (IKIteration = 0; IKIteration < maxFABRIKIteration && glm::l2Norm(ik.EndEffectorPosition() - EndPosition) > eps; IKIteration++) {
            // task: fabr ik
            // backward update
            glm::vec3 next_position         = EndPosition;
            backward_positions[nJoints - 1] = EndPosition;

            for (int i = nJoints - 2; i >= 0; i--) {
                // your code here
                glm::vec3 dir = glm::normalize(backward_positions[i + 1] - ik.JointGlobalPosition[i]);
                backward_positions[i] = backward_positions[i + 1] - dir * ik.JointOffsetLength[i + 1];
            }

            // forward update
            glm::vec3 now_position = ik.JointGlobalPosition[0];
            forward_positions[0] = ik.JointGlobalPosition[0];
            for (int i = 0; i < nJoints - 1; i++) {
                // your code here
                glm::vec3 dir = glm::normalize(backward_positions[i + 1] - forward_positions[i]);
                forward_positions[i + 1] = forward_positions[i] + dir * ik.JointOffsetLength[i + 1];
            }
            ik.JointGlobalPosition = forward_positions; // copy forward positions to joint_positions
        }
        //printf("%d\n", IKIteration);

        // Compute joint rotation by position here.
        for (int i = 0; i < nJoints - 1; i++) {
            ik.JointGlobalRotation[i] = glm::rotation(glm::normalize(ik.JointLocalOffset[i + 1]), glm::normalize(ik.JointGlobalPosition[i + 1] - ik.JointGlobalPosition[i]));
        }
        ik.JointLocalRotation[0] = ik.JointGlobalRotation[0];
        for (int i = 1; i < nJoints - 1; i++) {
            ik.JointLocalRotation[i] = glm::inverse(ik.JointGlobalRotation[i - 1]) * ik.JointGlobalRotation[i];
        }
        ForwardKinematics(ik, 0);
    }

    IKSystem::Vec3ArrPtr IKSystem::BuildCustomTargetPosition() {
        // get function from https://www.wolframalpha.com/input/?i=Albert+Einstein+curve
        /*
        int nums = 5000;
        using Vec3Arr = std::vector<glm::vec3>;
        std::shared_ptr<Vec3Arr> custom(new Vec3Arr(nums));
        int index = 0;
        for (int i = 0; i < nums; i++) {
            float x_val = 1.5e-3f * custom_x(92 * glm::pi<float>() * i / nums);
            float y_val = 1.5e-3f * custom_y(92 * glm::pi<float>() * i / nums);
            if (std::abs(x_val) < 1e-3 || std::abs(y_val) < 1e-3) continue;
            glm::vec3 cur = glm::vec3(1.6f - x_val, 0.0f, y_val - 0.2f);
            if(i > 0 && glm::length(cur - (*custom)[index - 1]) < 0.1)
            {
                glm::vec3 last = (*custom)[index - 1];
                int n = int(glm::length(cur - last) / 0.03 + 0.9999);
                glm::vec3 step = (cur - last) / (1.0f * n);
                for(int j = 1; j <= n; j++)
                    (*custom)[index++] = last + step * (1.0f * j);
            }
            else
                (*custom)[index++] = cur;
        }
        custom->resize(index);
        return custom;
        */

        
        int iw, ih, n;
        unsigned char *idata = stbi_load("assets/images/paimon.jpg", &iw, &ih, &n, 0);
        unsigned char *odata = new unsigned char[iw * ih * n];
        float kernel1[3][3] = {{-1.0, 0, 1.0},
                               {-2.0, 0, 2.0},
                               {-1.0, 0, 1.0}};
        float kernel2[3][3] = {{ 1.0,  2.0,  1.0},
                               { 0.0,  0.0,  0.0},
                               {-1.0, -2.0, -1.0}};                 
        for (int x = 0; x + 2 < ih; ++x)
            for (int y = 0; y + 2 < iw; ++y)
            {
                glm::vec3 sum1 = {0, 0, 0}, sum2 = {0, 0, 0};
                for (std::size_t i = 0; i < 3; ++i)
                    for (std::size_t j = 0; j < 3; ++j)
                    {
                        glm::vec3 col = {idata[(x + i) * iw * 3 + (y + j) * 3 + 0], idata[(x + i) * iw * 3 + (y + j) * 3 + 1], idata[(x + i) * iw * 3 + (y + j) * 3 + 2]};
                        sum1 += kernel1[i][j] * col;
                        sum2 += kernel2[i][j] * col;
                    }
                for(int l = 0; l < 3; l++)
                    odata[x * iw * 3 + y * 3 + l] = sqrt(sum1[l] * sum1[l] + sum2[l] * sum2[l]);
            }
        //stbi_write_png("1.jpg", iw, ih, n, odata, 0);
        
        std::shared_ptr<Vec3Arr> custom(new Vec3Arr());
        Vec3Arr points;
        for (int x = 0; x < ih; ++x)
            for (int y = 0; y < iw; ++y)
            {
                glm::vec3 col = {odata[x * iw * 3 + y * 3 + 0], odata[x * iw * 3 + y * 3 + 1], odata[x * iw * 3 + y * 3 + 2]};;
                col /= 255.0;
                if(col.r + col.g + col.b > 1.5)
                    points.push_back({2.5f * x / ih - 1.25f, 0.0f, 2.5f * y / iw - 1.25f});
            }
        std::vector<int> index(points.size());
        std::vector<bool> vis(points.size());
        index[0] = 0;
        vis[0] = true;
        for(std::size_t i = 1; i < points.size(); i++)
        {
            float mn = 1e10;
            for(std::size_t j = 0; j < points.size(); j++)
            {
                if(vis[j])
                    continue;
                float dis = glm::length(points[j] - points[index[i - 1]]);
                if(dis < mn)
                {
                    mn = dis;
                    index[i] = j;
                }
            }
            vis[index[i]] = true;
        }
        for(std::size_t i = 0; i < index.size(); i++)
            (*custom).push_back(points[index[i]]);
        return custom;
    }

    float Calc_g(const MassSpringSystem & system, const Eigen::VectorXf & x, const Eigen::VectorXf  & y, float h) {
        auto ttt = (x - y).transpose() * system.Mass * (x - y);
        float res = ttt[0] / (2.0f * h * h);
        std::vector<glm::vec3> forces(system.Positions.size(), glm::vec3(0));
        for (auto const spring : system.Springs) {
            auto const p0 = spring.AdjIdx.first;
            auto const p1 = spring.AdjIdx.second;
            glm::vec3 const x01 = glm::vec3(x[p1 * 3 + 0], x[p1 * 3 + 1], x[p1 * 3 + 2])  - glm::vec3(x[p0 * 3 + 0], x[p0 * 3 + 1], x[p0 * 3 + 2]);
            float e = 0.5f * system.Stiffness * glm::pow(glm::length(x01) - spring.RestLength, 2);
            res += e;
        }
        return res;
    }
    Eigen::VectorXf Calc_grad_g(const MassSpringSystem & system, const Eigen::VectorXf & x, const Eigen::VectorXf  & y, float h) {
        Eigen::VectorXf res = (x - y) * system.Mass / h / h;
        for (auto const spring : system.Springs) {
            auto const p0 = spring.AdjIdx.first;
            auto const p1 = spring.AdjIdx.second;
            glm::vec3 const x01 = glm::vec3(x[p1 * 3 + 0], x[p1 * 3 + 1], x[p1 * 3 + 2])  - glm::vec3(x[p0 * 3 + 0], x[p0 * 3 + 1], x[p0 * 3 + 2]);
            glm::vec3 const e01 = glm::normalize(x01);
            glm::vec3 f = system.Stiffness * (glm::length(x01) - spring.RestLength) * e01;
            for(int i = 0; i < 3; i++) {
                res[p0 * 3 + i] -= f[i];
                res[p1 * 3 + i] += f[i];
            }
        }
        // for(int i=0; i< system.Positions.size(); i++)
        //     printf("(%.2f, %.2f, %.2f)", res[i*3+0],res[i*3+1],res[i*3+2]);
        // printf("\n");
        return res;
    }
    bool check_gradgrad(const MassSpringSystem & system, Eigen::SparseMatrix<float> &gradgrad_g, const Eigen::VectorXf &grad_g, const Eigen::VectorXf & x, const Eigen::VectorXf & y, float h, int n) {
        const float delta = 0.000001;
        bool flag = true;
        printf("Start check gradgrad\n");
        for(int j = 0; j < 3 * n; j++) {
            Eigen::VectorXf x1 = x;
            x1[j] += delta;
            Eigen::VectorXf new_grad_g = Calc_grad_g(system, x1, y, h);
            Eigen::VectorXf new_gradgrad = (new_grad_g - grad_g) / delta;
            for(int i = 0; i < 3 * n; i++)
            {
                float temp = gradgrad_g.coeffRef(i, j);
                if(glm::max(glm::abs(temp), glm::abs(new_gradgrad[i])) > 1e-4 && glm::abs(new_gradgrad[i] - temp) / glm::max(glm::abs(temp), glm::abs(new_gradgrad[i])) > 0.05)
                {
                    printf("Found error:\ngradgrad_g[%d][%d]\nExpected %.5f   Found %.5f\n", i, j, new_gradgrad[i], temp);
                    flag = false;
                }
            }
        }
        return flag;
    }
    void Calc_gradgrad_g(Eigen::SparseMatrix<float> &res, const MassSpringSystem & system, const Eigen::VectorXf & grad_g, const Eigen::VectorXf & x, const Eigen::VectorXf & y, float h) {
        std::vector<Eigen::Triplet<float>> coefficients;
        int n = system.Positions.size();

        // const float delta = 0.000001;
        // for(int j = 0; j < 3 * n; j++) {
        //     Eigen::VectorXf x1 = x;
        //     x1[j] += delta;
        //     Eigen::VectorXf new_grad_g = Calc_grad_g(system, x1, y, h);
        //     Eigen::VectorXf new_gradgrad = (new_grad_g - grad_g) / delta;
        //     for(int i = 0; i < 3 * n; i++)
        //         if(glm::abs(new_gradgrad[i]) > 1e-5)
        //         {
        //             coefficients.push_back(Eigen::Triplet(i, j, new_gradgrad[i]));
        //         }
        // }

        for(int i = 0; i < 3 * n; i++)
            coefficients.push_back(Eigen::Triplet(i, i, system.Mass / h / h));
        for (auto const spring : system.Springs) {
            int p0 = spring.AdjIdx.first;
            int p1 = spring.AdjIdx.second;
            glm::vec3 const x01 = glm::vec3(x[p1 * 3 + 0], x[p1 * 3 + 1], x[p1 * 3 + 2])  - glm::vec3(x[p0 * 3 + 0], x[p0 * 3 + 1], x[p0 * 3 + 2]);
            float dis = glm::length(x01);
            glm::mat3 A(0);
            for(int i = 0; i < 3; i++)
                A[i][i] = system.Stiffness * (1 - spring.RestLength / dis);
            float dis3 = dis * dis * dis;
            for(int i = 0; i < 3; i++)
                for(int j = 0; j < 3; j++)
                    A[i][j] += system.Stiffness * spring.RestLength * x01[i] * x01[j] / dis3;
            for(int i = 0; i < 3; i++)
                for(int j = 0; j < 3; j++)
                    if(glm::abs(A[i][j]) > 1e-5f)
                    {
                        coefficients.push_back(Eigen::Triplet(p0 * 3 + i, p1 * 3 + j, -A[i][j]));
                        coefficients.push_back(Eigen::Triplet(p1 * 3 + j, p0 * 3 + i, -A[j][i]));
                        coefficients.push_back(Eigen::Triplet(p0 * 3 + i, p0 * 3 + j, A[i][j]));
                        coefficients.push_back(Eigen::Triplet(p1 * 3 + j, p1 * 3 + i, A[j][i]));
                    }
        }
        res.setFromTriplets(coefficients.begin(), coefficients.end());
    }
    Eigen::SparseMatrix<float> Calc_Inv(const Eigen::SparseMatrix<float> &A, int n) {
        auto solver = Eigen::SimplicialLDLT<Eigen::SparseMatrix<float>>(A) ;
        Eigen::SparseMatrix<float> I(3 * n, 3 * n);
        I.setIdentity();
        Eigen::SparseMatrix<float> A_inv = solver.solve(I);
        // Eigen::SparseMatrix<float> res = A * A_inv;
        // for(int i = 0; i < 3 * n; i++)
        //     for(int j = 0; j < 3 * n; j++)
        //     {
        //         float temp = res.coeffRef(i, j);
        //         if(glm::abs(temp) < 1e-2)
        //             continue;
        //         printf("[%d][%d]=%.6f\n",i, j, temp);
        //     }
        return A_inv;
    }

    void AdvanceMassSpringSystem(MassSpringSystem & system, float const dt) {
        // your code here: rewrite following code
        bool const useNewtonMethod = true; // choose the algorithm

        int const numIteration = 3;
        int const steps = 1;
        float h = dt / steps; 
        float const beta = 0.9;
        float const gamma = 0.0001;

        // h = 0.07;
        // freopen("log.log", "a", stdout);
        // printf("delta time: %.5f\n", dt);
        for (std::size_t s = 0; s < steps; s++) {
            int n = system.Positions.size();
            Eigen::VectorXf x0 = Eigen::VectorXf::Zero(3 * n);
            Eigen::VectorXf v0 = Eigen::VectorXf::Zero(3 * n);
            Eigen::VectorXf grav = Eigen::VectorXf::Zero(3 * n);
            for(int i = 0; i < 3 * n; i++) {
                x0[i] = system.Positions[i / 3][i % 3];
                v0[i] = system.Velocities[i / 3][i % 3];
                grav[i] = i % 3 == 1 ? -system.Gravity : 0.0f;
            }
            Eigen::VectorXf y = x0 + v0 * h + grav * h * h;
            Eigen::VectorXf x1 = y;
            float g1 = Calc_g(system, x1, y, h);
            for(int iter = 0; iter < numIteration; iter++) {
                Eigen::VectorXf grad_g = Calc_grad_g(system, x1, y, h);

                Eigen::VectorXf delta_x;
                if(useNewtonMethod) {
                    Eigen::SparseMatrix<float> gradgrad_g(3 * n, 3 * n);
                    Calc_gradgrad_g(gradgrad_g, system, grad_g, x1, y, h);
                    // check_gradgrad(system, gradgrad_g, grad_g, x1, y, h, n);
                    delta_x = -Calc_Inv(gradgrad_g, n) * grad_g;
                }
                else
                    delta_x = -grad_g;
                
                // for(int i=0; i< system.Positions.size(); i++)
                //     printf("(%.2f, %.2f, %.2f)", delta_x[i*3+0],delta_x[i*3+1],delta_x[i*3+2]);
                // printf("\n");
                Eigen::VectorXf x2 = Eigen::VectorXf::Zero(3 * n);
                float alpha = 1.0f / beta;
                float g2, check;
                do {
                    alpha = beta * alpha;
                    x2 = x1 + delta_x * alpha;
                    g2 = Calc_g(system, x2, y, h);
                    check = g1 + gamma * alpha * (grad_g.transpose() * delta_x)[0];
                    //printf("alpha: %.5f  g1: %.5f  g2: %.5f  check: %.5f\n", alpha, g1, g2, check);
                }while(g2 > check + 1e-4);
                x1 = x2;
                g1 = g2;
                // printf("[iter]: %d  energy: %.5f\n", iter, g1);
            }
            Eigen::VectorXf v1 = (x1 - x0) / h;
            for(int i = 0; i < n; i++) {
                if (system.Fixed[i]) continue;
                system.Positions[i] = {x1[i * 3 + 0], x1[i * 3 + 1], x1[i * 3 + 2]};
                system.Velocities[i] = {v1[i * 3 + 0], v1[i * 3 + 1], v1[i * 3 + 2]};
            }
            //printf("{step}: %d\n", (int)s);
        }
    }
}
