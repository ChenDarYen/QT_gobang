import network
import numpy as np
import torch


def export(load_state_path, model_save_path):
    model = network.Model()
    model.load_state_dict(torch.load(load_state_path))

    tmp_input = torch.rand(1, 3, 15, 15)

    traced_script_model = torch.jit.trace(model, tmp_input)

    output_value, output_prob = model(tmp_input)
    output_value_, output_prob_ = traced_script_model(tmp_input)

    np.testing.assert_allclose(output_value.detach().numpy(), output_value_.detach().numpy())
    np.testing.assert_allclose(output_prob.detach().numpy(), output_prob_.detach().numpy())

    traced_script_model.save(model_save_path)


export('state/model_600.pkl', 'traced_model/model_600.pt')
